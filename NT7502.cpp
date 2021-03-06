/*
$Id:$

ST7565 LCD library!

Copyright (C) 2010 Limor Fried, Adafruit Industries

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

some of this code was written by <cstone@pobox.com> originally; it is in the public domain.

update by nopnop2002; it is in the public domain.
*/


#include <SPI.h>
#include "NT7502.h"
#ifdef __AVR__
#include <avr/pgmspace.h>
#include <util/delay.h>
#endif

#define _delay_ms(t) delay(t)
#define _BV(bit) (1<<(bit))
#define swap(a, b) { uint8_t t = a; a = b; b = t; }

#define _SHIFTOUT_
//#define _BITBANGING_
//#define _HARDSPI_
//#define _DEBUG_


// a handy reference to where the pages are on the screen
//const uint8_t pagemap[] = { 3, 2, 1, 0, 7, 6, 5, 4 };
//const uint8_t pagemap[] = { 7, 6, 5, 4, 3, 2, 1 };

// a 5x7 font table
const extern uint8_t PROGMEM font[];

// the memory buffer for the LCD
// NT7502 have 8Page * 132Column
// this module have only 7 page, so page 0 is not use
// this module have 112 dot, so colum 0-19 is not use
//
// +-------------------------------------+ ---
// |    |          page 0                |  | not use
// +-------------------------------------+ ---
// |    |          page 1                |  |
// +-------------------------------------+  |
// |    |          page 2                |  |
// +-------------------------------------+  |
// |    |          page 3                |  |
// +-------------------------------------+  | 8*7=56 Dot
// |    |          page 4                |  |
// +-------------------------------------+  |
// |    |          page 5                |  |
// +-------------------------------------+  |
// |    |          page 6                |  |
// +-------------------------------------+  |
// |    |          page 7                |  |
// +-------------------------------------+ --
// |    |                                |
// 0    20   <-------112 dot -------->   131

uint8_t st7565_buffer[8][132] = { 0 };


// expand font bit
inline void ST7565::expandfont(uint8_t ch, uint8_t bai, uint8_t *exfont) {
  uint8_t mask = 0x80;
  uint8_t pos = 0;
  uint8_t shift = 0;
  
  for(int i=0;i<8;i++) {
//    Serial.print("mask=");
//    Serial.print(mask,HEX);
    for(int j=0;j<bai;j++) {
      exfont[pos] = exfont[pos] << 1;
//      Serial.print(" shift=");
//      Serial.print(shift);
      if (ch & mask) exfont[pos]++;
//      Serial.print(" pos=");
//      Serial.print(pos);
//      Serial.print(" ch2=");
//      Serial.print(exfont[pos],HEX);
      shift++;
      if (shift > 7) {
        shift = 0;
        pos++;
      }
    }
    mask = mask >> 1;
//    Serial.println();
  }
}

inline unsigned char ST7565::rotateByte(uint8_t ch1) {
  uint8_t ch2;
  for (int j=0;j<8;j++) {
    ch2 = (ch2 << 1) + (ch1 & 0x01);
    ch1 = ch1 >> 1;
  }
  return ch2;
}


void ST7565::drawUTF(uint8_t x, uint8_t line, 
			uint8_t *font, uint8_t w, uint8_t h) {
  if (line >= 7) return;
  if (x >= LCDWIDTH) return;
  uint8_t _p = line+1;
  uint8_t _x = x + OFFSET;
  for (uint8_t yy=0; yy<(h/8); yy++) {
    for (uint8_t xx=0; xx<w; xx++ ) {
#ifdef _DEBUG_
      Serial.print("_p+yy=");
      Serial.print(_p+yy);
      Serial.print(" _x+xx");
      Serial.print(_x+xx);
      Serial.print(" font=");
      Serial.print(font[yy*32+xx],HEX);
      Serial.println();
      delay(10);
#endif
	  if (_p+yy <= 7 && _x+xx <= 131) {
        uint8_t ch1 = font[yy*32+xx];
        uint8_t ch2;
        ch2 = rotateByte(ch1);
		st7565_buffer[_p+yy][_x+xx] = ch2;
      }
    }
  }
}


void ST7565::drawbitmap(uint8_t x, uint8_t y, 
			const uint8_t *bitmap, uint8_t w, uint8_t h,
			uint8_t color) {
  for (uint8_t j=0; j<h; j++) {
    for (uint8_t i=0; i<w; i++ ) {
      if (pgm_read_byte(bitmap + i + (j/8)*w) & _BV(j%8)) {
	    setpixel(x+i, y+j, color);
      }
    }
  }
}

void ST7565::drawstring(uint8_t x, uint8_t line, char *c) {

  while (c[0] != 0) {
    drawchar(x, line, 1, c[0]);
    c++;
    x += 6; // 6 pixels wide
    if (x + 6 >= LCDWIDTH) {
      x = 0;    // ran out of this line
      line++;
    }
    if (line >= (LCDHEIGHT/8))
      return;        // ran out of space :(
  }
}

void ST7565::drawstring_P(uint8_t x, uint8_t line, const char *str) {

  while (1) {
    char c = pgm_read_byte(str++);
    if (! c)
      return;
    drawchar(x, line, 1, c);
    x += 6; // 6 pixels wide
    if (x + 6 >= LCDWIDTH) {
      x = 0;    // ran out of this line
      line++;
    }
    if (line >= (LCDHEIGHT/8))
      return;        // ran out of space :(
  }
}

void  ST7565::drawchar(uint8_t x, uint8_t line, uint8_t bai, char c) {

  if (line >= 7) return;
  if (x >= LCDWIDTH) return;
  uint8_t _p = line+1;
  uint8_t _x = x + OFFSET;
  uint8_t exfont[8];

  for (uint8_t i=0; i<5; i++ ) {
	uint8_t ch1 = pgm_read_byte(font+(c*5)+i);
    uint8_t ch2;
    ch2 = rotateByte(ch1);
#if 0
	for (int j=0;j<8;j++) {
      ch2 = (ch2 << 1) + (ch1 & 0x01);
      ch1 = ch1 >> 1;
    }
#endif
//    uint8_t bai=2;
#ifdef _DEBUG_
Serial.println(ch2,HEX);
#endif
    if (bai == 1) {
      st7565_buffer[_p][_x] = ch2;
      _x++;
    } else {
//  Expand Font
      expandfont(ch2, bai, exfont);
#ifdef _DEBUG_
for(int f=0;f<bai;f++) {
  Serial.print(" font=");
  Serial.print(exfont[f],HEX); 
}
Serial.println();
#endif

      uint8_t _bai = bai-1;
      for(int ex1=0;ex1<bai;ex1++) {
        for(int ex2=0;ex2<bai;ex2++) {
          st7565_buffer[_p+ex1][_x+ex2] = exfont[_bai];
        }
        _bai--;
      }
      _x+=bai;
    }
  }
}

void  ST7565::linedown(void) {
  for(int page=6;page>0;page--) {
	memcpy(&st7565_buffer[page+1], &st7565_buffer[page], VRAMCOL);
  }
  memset(&st7565_buffer[0], 0, VRAMCOL);
}

void  ST7565::lineup(void) {
  for(int page=2;page<8;page++) {
	memcpy(&st7565_buffer[page-1], &st7565_buffer[page], VRAMCOL);
  }
  memset(&st7565_buffer[7], 0, VRAMCOL);
}

// bresenham's algorithm - thx wikpedia
void ST7565::drawline(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color) {

  uint8_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  uint8_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int8_t err = dx / 2;
  int8_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;}

  for (; x0<=x1; x0++) {
    if (steep) {
      setpixel(y0, x0, color);
    } else {
      setpixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

// filled rectangle
void ST7565::fillrect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color) {

  // stupidest version - just pixels - but fast with internal buffer!
  for (uint8_t i=x; i<x+w; i++) {
    for (uint8_t j=y; j<y+h; j++) {
      setpixel(i, j, color);
    }
  }
}

// draw a rectangle
void ST7565::drawrect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color) {

  // stupidest version - just pixels - but fast with internal buffer!
  for (uint8_t i=x; i<x+w; i++) {
    setpixel(i, y, color);
    setpixel(i, y+h-1, color);
  }

  for (uint8_t i=y; i<y+h; i++) {
    setpixel(x, i, color);
    setpixel(x+w-1, i, color);
  } 
}

// draw a circle outline
void ST7565::drawcircle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color) {

  int8_t f = 1 - r;
  int8_t ddF_x = 1;
  int8_t ddF_y = -2 * r;
  int8_t x = 0;
  int8_t y = r;

  setpixel(x0, y0+r, color);
  setpixel(x0, y0-r, color);
  setpixel(x0+r, y0, color);
  setpixel(x0-r, y0, color);

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
  
    setpixel(x0 + x, y0 + y, color);
    setpixel(x0 - x, y0 + y, color);
    setpixel(x0 + x, y0 - y, color);
    setpixel(x0 - x, y0 - y, color);
    
    setpixel(x0 + y, y0 + x, color);
    setpixel(x0 - y, y0 + x, color);
    setpixel(x0 + y, y0 - x, color);
    setpixel(x0 - y, y0 - x, color);
    
  }
}

void ST7565::fillcircle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color) {

  int8_t f = 1 - r;
  int8_t ddF_x = 1;
  int8_t ddF_y = -2 * r;
  int8_t x = 0;
  int8_t y = r;

  for (uint8_t i=y0-r; i<=y0+r; i++) {
    setpixel(x0, i, color);
  }

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
  
    for (uint8_t i=y0-y; i<=y0+y; i++) {
      setpixel(x0+x, i, color);
      setpixel(x0-x, i, color);
    } 
    for (uint8_t i=y0-x; i<=y0+x; i++) {
      setpixel(x0+y, i, color);
      setpixel(x0-y, i, color);
    }    
  }
}

// the most basic function, set a single pixel
void ST7565::setpixel(uint8_t x, uint8_t y, uint8_t color) {

  if ((x >= LCDWIDTH) || (y >= LCDHEIGHT))
    return;
  uint8_t _x = x + OFFSET; // colum = 20-131(0-19 is not use)
  uint8_t _p = (63 - y)/8; // page = 1-7(page 0 is not use)

#ifdef _DEBUG_
Serial.print("_x:");
Serial.print(_x);
Serial.print(" y:");
Serial.print(y);
Serial.print(" _p:");
Serial.println(_p);
#endif

  // x is which column
  if (color) 
    st7565_buffer[_p][_x] |= _BV(7-(y%8));  
  else
    st7565_buffer[_p][_x] &= ~_BV(7-(y%8)); 
}

// the most basic function, get a single pixel
uint8_t ST7565::getpixel(uint8_t x, uint8_t y) {

  if ((x >= LCDWIDTH) || (y >= LCDHEIGHT))
    return 0;
  uint8_t _x = x + OFFSET;
  uint8_t _p = (63 - y)/8;

  return (st7565_buffer[_p][_x] >> (7-(y%8))) & 0x1;  
}

void ST7565::begin(uint8_t contrast) {
#if defined (_SHIFTOUT_)
Serial.println("shiftOut");
#endif
#if defined (_BITBANGING_)
Serial.println("bitBanging SPI");
#endif
#if defined (_HARDSPI_)
Serial.println("hardware SPI");
  SPI.begin();
#endif
  st7565_init();
  st7565_command(CMD_DISPLAY_ON);
  st7565_command(CMD_SET_ALLPTS_NORMAL);
  set_brightness(contrast);
}

void ST7565::st7565_init(void) {
  // set pin directions
  pinMode(sid, OUTPUT);
  pinMode(sclk, OUTPUT);
  pinMode(a0, OUTPUT);
  pinMode(rst, OUTPUT);
  pinMode(cs, OUTPUT);

  // toggle RST low to reset; CS low so it'll listen to us
  if (cs > 0)
    digitalWrite(cs, LOW);

  digitalWrite(rst, LOW);
  _delay_ms(500);
  digitalWrite(rst, HIGH);

  // LCD bias select
  st7565_command(CMD_SET_BIAS_7);
//  st7565_command(CMD_SET_BIAS_9);
  // ADC select
  st7565_command(CMD_SET_ADC_NORMAL);
  // SHL select
  st7565_command(CMD_SET_COM_NORMAL);
  // Initial display line
  st7565_command(CMD_SET_DISP_START_LINE);

  // turn on voltage converter (VC=1, VR=0, VF=0)
  st7565_command(CMD_SET_POWER_CONTROL | 0x4);
  // wait for 50% rising
  _delay_ms(50);

  // turn on voltage regulator (VC=1, VR=1, VF=0)
  st7565_command(CMD_SET_POWER_CONTROL | 0x6);
  // wait >=50ms
  _delay_ms(50);

  // turn on voltage follower (VC=1, VR=1, VF=1)
  st7565_command(CMD_SET_POWER_CONTROL | 0x7);
  // wait
  _delay_ms(10);

  // set lcd operating voltage (regulator resistor, ref voltage resistor)
  st7565_command(CMD_SET_RESISTOR_RATIO | 0x6);

}


inline void ST7565::spiwrite(uint8_t c) {
    
#if defined (_SHIFTOUT_)
  shiftOut(sid, sclk, MSBFIRST, c);
#endif

#if defined (_BITBANGING_)
  int8_t i;
  for (i=7; i>=0; i--) {
    digitalWrite(sclk, LOW);
//    delayMicroseconds(5);      //need to slow down the data rate for Due and Zero
    if (c & _BV(i))
      digitalWrite(sid, HIGH);
    else
      digitalWrite(sid, LOW);
//     delayMicroseconds(5);      //need to slow down the data rate for Due and Zero
    digitalWrite(sclk, HIGH);
  }
#endif

#if defined (_HARDSPI_)
  SPI.transfer(c);
#endif
}


void ST7565::st7565_command(uint8_t c) {
  digitalWrite(a0, LOW);
  spiwrite(c);
}

void ST7565::st7565_data(uint8_t c) {
  digitalWrite(a0, HIGH);
  spiwrite(c);
}
void ST7565::set_brightness(uint8_t val) {
  st7565_command(CMD_SET_VOLUME_FIRST);
  st7565_command(CMD_SET_VOLUME_SECOND | (val & 0x3f));
}

void ST7565::display(void) {
  for(uint8_t page = 1; page < 8; page++) {

    st7565_command(CMD_SET_PAGE | page);

//    st7565_command(CMD_SET_COLUMN_LOWER | ((ST7565_STARTBYTES) & 0xf));
//    st7565_command(CMD_SET_COLUMN_UPPER | (((ST7565_STARTBYTES) >> 4) & 0x0F));
    st7565_command(CMD_SET_COLUMN_LOWER);
    st7565_command(CMD_SET_COLUMN_UPPER);
    st7565_command(CMD_RMW);
    
    for(int col=0; col< VRAMCOL; col++) {
      st7565_data(st7565_buffer[page][col]);
    }
  }
}

// clear everything
void ST7565::clear(void) {
  memset(st7565_buffer, 0, sizeof(st7565_buffer));
}

// this doesnt touch the buffer, just clears the display RAM - might be handy
void ST7565::clear_display(void) {

  for(uint8_t page = 1; page < 8; page++) {
    st7565_command(CMD_SET_PAGE | page);

//    st7565_command(CMD_SET_COLUMN_LOWER | ((ST7565_STARTBYTES) & 0xf));
//    st7565_command(CMD_SET_COLUMN_UPPER | (((ST7565_STARTBYTES) >> 4) & 0x0F));
    st7565_command(CMD_SET_COLUMN_LOWER);
    st7565_command(CMD_SET_COLUMN_UPPER);
    st7565_command(CMD_RMW);

    for(int col=0; col< VRAMCOL; col++) {
      st7565_data(0x0);
    }
  }
}

