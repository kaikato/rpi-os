#include "io.h"
#include "fb.h"
#include "mb.h"
#include "terminal.h"
#include "printf.h"

unsigned int width, height, pitch, isrgb;
unsigned char *fb;

void fb_init()
{
    mbox[0] = 35*4; // Length of message in bytes
    mbox[1] = MBOX_REQUEST;

    mbox[2] = MBOX_TAG_SETPHYWH; // Tag identifier
    mbox[3] = 8; // Value size in bytes
    mbox[4] = 0;
    mbox[5] = 1920; // Value(width)
    mbox[6] = 1080; // Value(height)
    //mbox[5] = 480; // Value(width)
    //mbox[6] = 640; // Value(height)

    mbox[7] = MBOX_TAG_SETVIRTWH;
    mbox[8] = 8;
    mbox[9] = 8;
    mbox[10] = 1920;
    mbox[11] = 1080;
    //mbox[10] = 480;
    //mbox[11] = 640;

    mbox[12] = MBOX_TAG_SETVIRTOFF;
    mbox[13] = 8;
    mbox[14] = 8;
    mbox[15] = 0; // Value(x)
    mbox[16] = 0; // Value(y)

    mbox[17] = MBOX_TAG_SETDEPTH;
    mbox[18] = 4;
    mbox[19] = 4;
    mbox[20] = 32; // Bits per pixel ( bytes_per_pixel = 4 )

    mbox[21] = MBOX_TAG_SETPXLORDR; 
    mbox[22] = 4;
    mbox[23] = 4;   
    mbox[24] = 1; // 0x1 = RGB and 0x0 = BGR

    mbox[25] = MBOX_TAG_GETFB;
    mbox[26] = 8;    // value buffer size = 8 bytes ... on return get 4 byte addr to buffer and 4 byte size of buffer
    mbox[27] = 8;    // this number seems to get ignored but should be 0 
    mbox[28] = 4096; // FrameBufferInfo.pointer (buffer that is 4096 byte aligned ... on return should contain an address)
    mbox[29] = 0;    // FrameBufferInfo.size    (size of frame buffer in bytes height * width * bytes_per_pixel)

    mbox[30] = MBOX_TAG_GETPITCH;
    mbox[31] = 4;
    mbox[32] = 4;
    mbox[33] = 0; // Bytes per line

    //mbox[30] = 0x00040007; // check if alpha mode is enabled (0x0 means enabled 0x2 means ignored)
    //mbox[31] = 4;          // return value is a 4 byte value
    //mbox[32] = 0;          // should be 0 since this is a message 
    //mbox[33] = 0;          // set this to 0 and it gets written to by the hardware 

    mbox[34] = MBOX_TAG_LAST;

    // Check call is successful and we have a pointer with depth 32
    if (mbox_call(MBOX_CH_PROP) && mbox[20] == 32 && mbox[28] != 0) {
        mbox[28] &= 0x3FFFFFFF;  // Convert GPU address to ARM address ( subtracts 0x40000000 from address )
        width  = mbox[10];       // Actual physical width
        height = mbox[11];       // Actual physical height
        pitch  = mbox[33];       // Number of bytes per line
        isrgb  = mbox[24];       // Pixel order
        fb = (unsigned char *)((long)mbox[28]);

        unsigned int FRAMEBUFFER_SIZE = mbox[29];
    }
}

void drawImage(unsigned char* image) {
//    memcpy(fb, image, nbytes);
//    *fb = image;
}

void drawPixel(int x, int y, unsigned int attr)
{
    int offs = (y * pitch) + (x * 4); // i think 4 because the bytes per pixel is equal to 4
    //*((unsigned int*)(fb + offs)) = vgapal[attr & 0x0f];
    *((unsigned int*)(fb + offs)) = attr;
}

void drawPixelArray(unsigned char* image, int x, int y, unsigned int attr) {
    int offs = (y * pitch) + (x * 4); // i think 4 because the bytes per pixel is equal to 4
    *((unsigned int*)(image + offs)) = attr;
}

void drawRect(int x1, int y1, int x2, int y2, unsigned char attr, int fill)
{
    int y=y1;

    while (y <= y2) {
       int x=x1;
       while (x <= x2) {
	  if ((x == x1 || x == x2) || (y == y1 || y == y2)) drawPixel(x, y, attr);
	  else if (fill) drawPixel(x, y, (attr & 0xf0) >> 4);
          x++;
       }
       y++;
    }
}

void drawLine(int x1, int y1, int x2, int y2, unsigned char attr)  
{  
    int dx, dy, p, x, y;

    dx = x2-x1;
    dy = y2-y1;
    x = x1;
    y = y1;
    p = 2*dy-dx;

    while (x<x2) {
       if (p >= 0) {
          drawPixel(x,y,attr);
          y++;
          p = p+2*dy-2*dx;
       } else {
          drawPixel(x,y,attr);
          p = p+2*dy;
       }
       x++;
    }
}

void drawCircle(int x0, int y0, int radius, unsigned char attr, int fill)
{
    int x = radius;
    int y = 0;
    int err = 0;
 
    while (x >= y) {
	if (fill) {
	   drawLine(x0 - y, y0 + x, x0 + y, y0 + x, (attr & 0xf0) >> 4);
	   drawLine(x0 - x, y0 + y, x0 + x, y0 + y, (attr & 0xf0) >> 4);
	   drawLine(x0 - x, y0 - y, x0 + x, y0 - y, (attr & 0xf0) >> 4);
	   drawLine(x0 - y, y0 - x, x0 + y, y0 - x, (attr & 0xf0) >> 4);
	}
	drawPixel(x0 - y, y0 + x, attr);
	drawPixel(x0 + y, y0 + x, attr);
	drawPixel(x0 - x, y0 + y, attr);
        drawPixel(x0 + x, y0 + y, attr);
	drawPixel(x0 - x, y0 - y, attr);
	drawPixel(x0 + x, y0 - y, attr);
	drawPixel(x0 - y, y0 - x, attr);
	drawPixel(x0 + y, y0 - x, attr);

	if (err <= 0) {
	    y += 1;
	    err += 2*y + 1;
	}
 
	if (err > 0) {
	    x -= 1;
	    err -= 2*x + 1;
	}
    }
}

void drawChar(unsigned char ch, int x, int y, unsigned char attr)
{
    unsigned char *glyph = (unsigned char *)&font + (ch < FONT_NUMGLYPHS ? ch : 0) * FONT_BPG;

    for (int i=0;i<FONT_HEIGHT;i++) {
	for (int j=0;j<FONT_WIDTH;j++) {
	    unsigned char mask = 1 << j;
	    unsigned char col = (*glyph & mask) ? attr & 0x0f : (attr & 0xf0) >> 4;

	    drawPixel(x+j, y+i, col);
	}
	glyph += FONT_BPL;
    }
}

void drawString(int x, int y, char *s, unsigned char attr)
{
    while (*s) {
       if (*s == '\r') {
          x = 0;
       } else if(*s == '\n') {
          x = 0; y += FONT_HEIGHT;
       } else {
	  drawChar(*s, x, y, attr);
          x += FONT_WIDTH;
       }
       s++;
    }
}
