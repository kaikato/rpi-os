/*
 * Implement the following routines to set GPIO pins to input or output,
 * and to read (input) and write (output) them.
 *
 * DO NOT USE loads and stores directly: only use GET32 and PUT32
 * to read and write memory.  Use the minimal number of such calls.
 *
 * See rpi.h in this directory for the definitions.
 */
#include "rpi.h"

// see broadcomm documents for magic addresses.
enum {
    GPIO_BASE = 0x20200000,
    gpio_set0  = (GPIO_BASE + 0x1C),
    gpio_clr0  = (GPIO_BASE + 0x28),
    gpio_lev0  = (GPIO_BASE + 0x34),
    gpio_eds0  = (GPIO_BASE + 0x40),
    gpio_eds1  = (GPIO_BASE + 0x44),
    gpio_ren0 = (GPIO_BASE + 0x4c),
    gpio_ren1 = (GPIO_BASE + 0x50),
    gpio_fen0 = (GPIO_BASE + 0x58),
    gpio_fen1 = (GPIO_BASE + 0x5c)
};

//
// Part 1 implement gpio_set_on, gpio_set_off, gpio_set_output
//

void gpio_set_function(unsigned pin, gpio_func_t function){
  if(pin >= 32)
    return;
  if(function > 7)
    return;
  unsigned reg = GPIO_BASE + (pin/10)*0x4;
  unsigned shift = (pin%10)*3;
  unsigned read = GET32(reg);
  unsigned mask = (0x07 << shift);
  unsigned value = (function << shift);
  read &= ~mask;
  read |= value;
  PUT32(reg, read);
}

// set <pin> to be an output pin.
//
// note: fsel0, fsel1, fsel2 are contiguous in memory, so you
// can (and should) use array calculations!
void gpio_set_output(unsigned pin) {
    if(pin >= 32)
        return;
  gpio_set_function(pin, GPIO_FUNC_OUTPUT);
  // implement this
  // use <gpio_fsel0>
  /*
  unsigned reg = GPIO_BASE + (pin/10)*0x4;
  unsigned shift = (pin%10)*3;
  unsigned read = GET32(reg);
  unsigned mask = (0x07 << shift);
  unsigned value = (0x01 << shift);
  read &= ~mask;
  read |= value;
  PUT32(reg, read);
  */
}

// set GPIO <pin> on.
void gpio_set_on(unsigned pin) {
    if(pin >= 32)
        return;
  // implement this
  // use <gpio_set0>
  PUT32(gpio_set0, (1<<pin));
}

// set GPIO <pin> off
void gpio_set_off(unsigned pin) {
    if(pin >= 32)
        return;
  // implement this
  // use <gpio_clr0>
  PUT32(gpio_clr0, (1<<pin));
}

// set <pin> to <v> (v \in {0,1})
void gpio_write(unsigned pin, unsigned v) {
    if(v)
        gpio_set_on(pin);
    else
        gpio_set_off(pin);
}

//
// Part 2: implement gpio_set_input and gpio_read
//

// set <pin> to input.
void gpio_set_input(unsigned pin) {
  // implement.
  if(pin >= 32)
        return;
  gpio_set_function(pin, GPIO_FUNC_INPUT);
  /*
  unsigned reg = GPIO_BASE + (pin/10)*4;
  unsigned shift = (pin%10)*3;
  unsigned read = GET32(reg);
  unsigned mask = (0x07 << shift);
  unsigned value = (0x00 << shift);
  read &= ~mask;
  read |= value;
  PUT32(reg, read);
  */
}

// return the value of <pin>
int gpio_read(unsigned pin) {
  unsigned v = 0;

  // implement.
  unsigned read = GET32(gpio_lev0);
  unsigned mask = 0x1 << pin;
  read &= mask;
  if (read >> pin == 0x1){
    v = 1;
  }
  return DEV_VAL32(v);
}