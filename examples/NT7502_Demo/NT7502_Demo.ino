/*
 *  ETW-11256K1表示デモ 
 */
#include <NT7502.h> // https://github.com/nopnop2002/NT7502-LCD
//#include <ST7565.h> // https://github.com/adafruit/ST7565-LCD

// the LCD backlight is connected up to a pin so you can turn it on & off
#define BACKLIGHT_LED 4

// Software SPIを使う場合　全てのピンは任意
// pin11 - Serial data out (MOSI)
// pin13 - Serial clock out (SCK)
// pin 7 - Data/Command select (RS or A0)
// pin 6 - LCD reset (RST)
// pin 5 - LCD chip select (CS)
ST7565 glcd(11, 13, 7, 6, 5); // Software SPI for Arduino
//ST7565 glcd(5, 14, 12, 13, 15); // Software SPI for ESP8266

// Hardtware SPIを使う場合　MOSIとSCKは固定
// pin11 - Serial data out (MOSI)
// pin13 - Serial clock out (SCK)
// pin 7 - Data/Command select (RS or A0)
// pin 6 - LCD reset (RST)
// pin 5 - LCD chip select (CS)
//ST7565 glcd(0, 0, 7, 6, 5); // Hardwre SPI for Arduino


#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 


#define xmin 0
#define xmax 111
#define ymin 0
#define ymax 55
 
// a bitmap of a 16x16 fruit icon
const static unsigned char __attribute__ ((progmem)) logo16_glcd_bmp[]={
0x30, 0xf0, 0xf0, 0xf0, 0xf0, 0x30, 0xf8, 0xbe, 0x9f, 0xff, 0xf8, 0xc0, 0xc0, 0xc0, 0x80, 0x00, 
0x20, 0x3c, 0x3f, 0x3f, 0x1f, 0x19, 0x1f, 0x7b, 0xfb, 0xfe, 0xfe, 0x07, 0x07, 0x07, 0x03, 0x00, };

// The setup() method runs once, when the sketch starts
void setup()   {                
  Serial.begin(9600);

  // turn on backlight
  pinMode(BACKLIGHT_LED, OUTPUT);
  digitalWrite(BACKLIGHT_LED, HIGH);
  glcd.begin(0);
  glcd.clear();

  for (int cont=0;cont<13;cont++) {
    glcd.st7565_set_brightness(cont);
    glcd.clear();
    int cont1 = cont/10;
    int cont2 = cont % 10;
    Serial.print("contrast=");
    Serial.print(cont1);
    Serial.println(cont2);
    glcd.drawstring(0, 0, "Please check");
    glcd.drawstring(0, 1, "contrast");
    glcd.drawchar(30, 3, 3, cont1+48); // X3
    glcd.drawchar(50, 3, 3, cont2+48); // X3

    glcd.display();
    delay(2000);
  }

  glcd.st7565_set_brightness(5);
  glcd.clear();

  // draw many lines
  for (int pass=0;pass<3;pass++) {
    testdrawline(pass);
    glcd.display();
    delay(2000);
  }

  // draw rectangles
  testdrawrect();
  glcd.display();
  delay(2000);

  // draw multiple rectangles
  glcd.clear();
  for (int pass=0;pass<5;pass++) {
    testfillrect(pass);
    glcd.display();
    delay(2000);
  }

  // draw mulitple circles
  glcd.clear();
  testdrawcircle();
  glcd.display();
  delay(2000);

  // draw a black circle, 10 pixel radius, at location (32,32)
  glcd.clear();
  glcd.fillcircle(xmin+10, ymin+10, 10, BLACK);
  glcd.fillcircle(xmin+10, ymax-10, 10, BLACK);
  glcd.fillcircle(xmax-10, ymin+10, 10, BLACK);
  glcd.fillcircle(xmax-10, ymax-10, 10, BLACK);
  glcd.display();
  delay(2000);

  // draw the first ~126 characters in the font
  glcd.clear();
  testdrawchar();
  glcd.display();
  delay(5000);

#if 0
  glcd.clear();
  glcd.drawbitmap(40, 20, logo16_glcd_bmp, 
  LOGO16_GLCD_HEIGHT, LOGO16_GLCD_WIDTH, BLACK);
  glcd.display();
  
  // draw a string at location (0,0)
  glcd.clear();
  glcd.drawstring(0, 0, "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation");
  glcd.display();
  delay(2000);
#endif

  glcd.clear();
  // draw a bitmap icon and 'animate' movement
  testdrawbitmap(logo16_glcd_bmp, LOGO16_GLCD_HEIGHT, LOGO16_GLCD_WIDTH);

}


void loop()                     
{}


#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

void testdrawbitmap(const uint8_t *bitmap, uint8_t w, uint8_t h) {
  int8_t icons[NUMFLAKES][3];
  randomSeed(666);     // whatever seed
 
  // initialize
  for (uint8_t f=0; f< NUMFLAKES; f++) {
    icons[f][XPOS] = random(xmax);
    icons[f][YPOS] = ymax;
    icons[f][DELTAY] = random(5) + 1;
  }

  while (1) {
    // draw each icon
    for (int f=0; f< NUMFLAKES; f++) {
      glcd.drawbitmap(icons[f][XPOS], icons[f][YPOS], logo16_glcd_bmp, w, h, BLACK);
    }
    glcd.display();
    delay(200);
    
    // then erase it + move it
    for (uint8_t f=0; f< NUMFLAKES; f++) {
      glcd.drawbitmap(icons[f][XPOS], icons[f][YPOS],  logo16_glcd_bmp, w, h, WHITE);
      // move it
      icons[f][YPOS] -= icons[f][DELTAY];
      // if its gone, reinit
      if (icons[f][YPOS] < ymin) {
      	icons[f][XPOS] = random(xmax);
	      icons[f][YPOS] = ymax;
	      icons[f][DELTAY] = random(5) + 1;
      }
    }
  }
}


void testdrawchar(void) {
//  for (uint8_t i=0; i<126; i++) {
  for (uint8_t i=0; i<18*7; i++) {
    glcd.drawchar((i % 18) * 6, i/18, 1, i);
  }    
}

void testdrawcircle(void) {
  for (uint8_t i=0; i<22; i+=2) {
    glcd.drawcircle(56, 28, i, BLACK);
  }
}


void testdrawrect(void) {
//   glcd.drawrect(0+xmin, 0, xmax-xmin, ymax-0, BLACK);
//   glcd.drawrect(2+xmin, 2, xmax-(4+xmin), ymax-4, BLACK);
//   glcd.drawrect(4+xmin, 4, xmax-(8+xmin), ymax-8, BLACK);
   for (uint8_t i=0; i<22; i+=2) {
    glcd.drawrect(i+xmin, i, xmax-(i+i+xmin), ymax-(i+i), BLACK);
  }
}

void testfillrect(int pass) {
  if (pass == 0) {
    glcd.fillrect(xmin, 0, 20, 20, BLACK);
    glcd.fillrect(xmin, ymax-20, 20, 20, BLACK);
    glcd.fillrect(xmax-20, 0, 20, 20, BLACK);
    glcd.fillrect(xmax-20, ymax-20, 20, 20, BLACK);
  } else if (pass == 1) {
    glcd.fillrect(xmin, 0, 20, 20, WHITE);
  } else if (pass == 2) {
    glcd.fillrect(xmin, ymax-20, 20, 20, WHITE);
  } else if (pass == 3) {
    glcd.fillrect(xmax-20, 0, 20, 20, WHITE);
  } else if (pass == 4) {
    glcd.fillrect(xmax-20, ymax-20, 20, 20, WHITE);
  }
}

void testdrawline(int pass) {
  if (pass == 0) {
    for (uint8_t i=xmin; i<=xmax; i+=4) {
      glcd.drawline(xmin, 0, i, ymax, BLACK);
    }
    for (uint8_t i=0; i<=ymax; i+=4) {
      glcd.drawline(xmin, 0, xmax, i, BLACK);
    }
  } else if (pass == 1) {
    for (uint8_t i=xmin; i<=xmax; i+=4) {
      glcd.drawline(xmin, 0, i, ymax, WHITE);
    }
  } else if (pass == 2 ) {
    for (uint8_t i=0; i<=ymax; i+=4) {
      glcd.drawline(xmin, 0, xmax, i, WHITE);
    }
  }
}
