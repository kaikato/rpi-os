#ifndef FB_H
#define FB_H

extern unsigned int FRAMEBUFFER_SIZE;

void fb_init();
void drawPixel(int x, int y, unsigned int attr);
void drawImage(unsigned char* image);
void drawPixelArray(unsigned char* image, int x, int y, unsigned int attr);
void drawChar(unsigned char ch, int x, int y, unsigned char attr);
void drawString(int x, int y, char *s, unsigned char attr);
void drawRect(int x1, int y1, int x2, int y2, unsigned char attr, int fill);
void drawCircle(int x0, int y0, int radius, unsigned char attr, int fill);
void drawLine(int x1, int y1, int x2, int y2, unsigned char attr);

#endif