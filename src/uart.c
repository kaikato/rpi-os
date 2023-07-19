// implement:
//  void uart_init(void)
//
//  int uart_can_get8(void);
//  int uart_get8(void);
//
//  int uart_can_put8(void);
//  void uart_put8(uint8_t c);
//
//  int uart_tx_is_empty(void) {
//
// see that hello world works.
//
//
#include "rpi.h"

enum {
    AUX_ENABLES = 0x20215004,
    AUX_MU_IO_REG = 0x20215040,
    AUX_MU_IER_REG = 0x20215044,
    AUX_MU_IIR_REG = 0x20215048,
    AUX_MU_LCR_REG = 0x2021504c,
    AUX_MU_MCR_REG = 0x20215050,
    AUX_MU_LSR_REG = 0x20215054,
    AUX_MU_MSR_REG = 0x20215058,
    AUX_MU_CNTL_REG = 0x20215060,
    AUX_MU_STAT_REG = 0x20215064,
    AUX_MU_BAUD_REG = 0x20215068
};
// called first to setup uart to 8n1 115200  baud,
// no interrupts.
//  - you will need memory barriers, use <dev_barrier()>
//
//  later: should add an init that takes a baud rate.
void uart_init(void) {
    /*
    dev_barrier();
    uint32_t read = GET32(AUX_ENABLES);
    uint32_t mask = ~0x1;
    read &= mask;
    PUT32(AUX_ENABLES, read);
    */
    dev_barrier();

    // sets the gpio pins 
    gpio_set_function(14, GPIO_FUNC_ALT5);
    gpio_set_function(15, GPIO_FUNC_ALT5);
    dev_barrier();

    // enable UART
    uint32_t read = GET32(AUX_ENABLES);
    uint32_t mask = 0x1;
    read |= mask;
    PUT32(AUX_ENABLES, read);
    dev_barrier();

    // disables tx/rx
    PUT32(AUX_MU_CNTL_REG, 0);

    // disable interrupts
    //read = GET32(AUX_MU_IER_REG);
    //mask = 0b11;
    //read &= ~mask;
    PUT32(AUX_MU_IER_REG, 0);

    // set data size to 8
    //read = GET32(AUX_MU_LCR_REG);
    //mask = 0b11;
    //read |= mask;
    PUT32(AUX_MU_LCR_REG, 0b11);

    PUT32(AUX_MU_MCR_REG, 0);

    // clear FIFOs
    //read = GET32(AUX_MU_IIR_REG);
    //mask = 0b11<<1;
    //read |= mask;
    PUT32(AUX_MU_IIR_REG, 0b110);

    
    // set baud register to 270
    PUT32(AUX_MU_BAUD_REG, 0b100001110);

    // need to make no flow control

    // enable tx and rx
    //read = GET32(AUX_MU_CNTL_REG);
    //mask = 0b11;
    //read |= mask;
    PUT32(AUX_MU_CNTL_REG, 0b11);
    dev_barrier();
}

// disable the uart.
void uart_disable(void) {
    dev_barrier();
    uart_flush_tx();
    dev_barrier();
    uint32_t read = GET32(AUX_ENABLES);
    uint32_t mask = ~0x1;
    read &= mask;
    PUT32(AUX_ENABLES, read);
    dev_barrier();
}

// returns one byte from the rx queue, if needed
// blocks until there is one.
int uart_get8(void) {
    dev_barrier();
    while(uart_has_data() != 1){}
    int byte = GET32(AUX_MU_IO_REG);
	return byte;
    dev_barrier();
}

// 1 = space to put at least one byte, 0 otherwise.
int uart_can_put8(void) {
    uint32_t read = GET32(AUX_MU_STAT_REG);
    uint32_t mask = (0b1<<1);
    read &= mask;
    if ((read>>1) == 0) {
        return 0;
    } else {
        return 1;
    }
}

// put one byte on the tx qqueue, if needed, blocks
// until TX has space.
// returns < 0 on error.
int uart_put8(uint8_t c) {
    dev_barrier();
    while(uart_can_put8() != 1){}
    PUT32(AUX_MU_IO_REG, c);
    return 0;
    dev_barrier();
}

// simple wrapper routines useful later.

// 1 = at least one byte on rx queue, 0 otherwise
int uart_has_data(void) {
    uint32_t read = GET32(AUX_MU_STAT_REG);
    uint32_t mask = (0b1);
    read &= mask;
    if (read == 0) {
        return 0;
    } else {
        return 1;
    }
}

// return -1 if no data, otherwise the byte.
int uart_get8_async(void) { 
    if(!uart_has_data())
        return -1;
    return uart_get8();
}

// 1 = tx queue empty, 0 = not empty.
int uart_tx_is_empty(void) {
    dev_barrier();
    uint32_t read = GET32(AUX_MU_STAT_REG);
    uint32_t mask = (0b1<<9);
    read &= mask;
    if ((read>>9)==1){
        return 1;
    } else {
        return 0;
    }
    dev_barrier();
}

// flush out all bytes in the uart --- we use this when 
// turning it off / on, etc.
void uart_flush_tx(void) {
    dev_barrier();
    while(!uart_tx_is_empty());
    dev_barrier();
}
