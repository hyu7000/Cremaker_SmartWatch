#ifndef _FONT_H_
#define _FONT_H_

#include <stdint.h>

 typedef struct {
     const uint8_t *data;
     uint16_t width;
     uint16_t height;
     uint8_t dataSize;
     } tImage;
     
 typedef struct {
     long int code;
     const tImage *image;
     } tChar;

 typedef struct {
     int length;
     const tChar *chars;
     } tFont;

extern const tFont TimeNumber;

extern const tFont Font;

#endif