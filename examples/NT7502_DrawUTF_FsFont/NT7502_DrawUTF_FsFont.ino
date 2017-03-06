/*
 *  ESP8266でETW-11256K1に漢字を表示するサンプル 
 *  漢字フォントはSPIFFS上に置く
 */
#include <FS.h>
#include <Fontx.h> // https://github.com/h-nari/Fontx
#include <FsFontx.h> // https://github.com/h-nari/Fontx
#include <NT7502.h> // https://github.com/nopnop2002/NT7502-LCD
//#include <ST7565.h> // https://github.com/adafruit/ST7565-LCD

FsFontx fx16("/ILGH16XB.FNT","/ILGZ16XB.FNT");
FsFontx fx24("/ILGH24XB.FNT","/ILGZ24XB.FNT");
FsFontx fx32("/ILGH32XB.FNT","/ILGZ32XB.FNT");

struct UTFINFO {
  uint16_t utf;
  uint8_t wbit;
  uint8_t hbit;
  uint8_t wbyte;
  uint8_t hbyte;
  uint8_t font[128];  // 32Dot X 32Dot
};

struct UTFINFO utfinfo;

// the LCD backlight is connected up to a pin so you can turn it on & off
#define BACKLIGHT_LED 4

// Software SPI (ESP8266)
// GPIO5  - Serial data out (MOSI)
// GPIO14 - Serial clock out (SCK)
// GPIO12 - Data/Command select (RS or A0)
// GPIO13 - LCD reset (RST)
// GPIO15 - LCD chip select (CS)
ST7565 glcd(5, 14, 12, 13, 15); // Software SPI

void dumpFontUTF(struct UTFINFO* hoge) {
  Serial.print("utf:");
  Serial.print(hoge->utf,HEX);
  Serial.print(" wbit:");
  Serial.print(hoge->wbit);
  Serial.print(" hbit:");
  Serial.print(hoge->hbit);
  Serial.print(" wbyte:");
  Serial.print(hoge->wbyte);
  Serial.print(" hbyte:");
  Serial.print(hoge->hbyte);
  Serial.println();

  for(int y=0; y<(hoge->hbit)/8; y++){
    Serial.printf("%02d: ",y);
    for(int x=0; x<hoge->wbit; x++){
      uint8_t d = hoge->font[y*32+x];
      Serial.print(d,HEX); 
      Serial.print(" "); 
    }
    Serial.println();
    delay(10);
  }

}

void getFontUTF(FsFontx fontx, uint16_t utf, struct UTFINFO* hoge) {
  const uint8_t *p;
  uint8_t w,h;

//  Serial.print("utf:");
//  Serial.println(utf,HEX);
  hoge->utf = utf;
  if(!fontx.getGlyph(utf, &p, &w, &h)){
    Serial.printf("getGlyph failed. code:%x\n",utf);
  } else {
    hoge->wbit = w;
    hoge->hbit = h;
    hoge->wbyte = w/8;
    hoge->hbyte = h/8;

    for(int y=0; y<(h/8); y++){
      for(int x=0; x<w; x++){
        hoge->font[y*32+x] = 0;
      }
    }

    int mask = 7;
    for(int y=0; y<h; y++){
//      Serial.printf("%02d: ",y);
      for(int x=0; x<w; x++){
        uint8_t d = pgm_read_byte(&p[x/8]);
//        Serial.print(d & (0x80 >> (x % 8)) ? '*' : '.');
        uint8_t pos = (y/8)*32+x;
        if (d & (0x80 >> (x % 8))) hoge->font[pos] = hoge->font[pos] + (1 << mask); 
      }
//      Serial.println();
//      delay(10);
//      dumpFontUTF(&utfinfo);
      mask--;
      if (mask < 0) mask = 7;
      p += (w + 7)/8;
    }
  }
}


// The setup() method runs once, when the sketch starts
void setup()   {                
  uint16_t str[] = { u'漢', u'字', u'T', u'E', u'S', u'T'};

  Serial.begin(9600);

  if(!SPIFFS.begin()) {
    Serial.println("SPIFFS failed.");
    return;
  } else {
    Dir dir = SPIFFS.openDir("/");
    int cnt = 0;
    while(dir.next()){
      File f = dir.openFile("r");
      Serial.printf("[%d] %-12s %12u\n",++cnt,f.name(), f.size());
      f.close();
    }
    Serial.printf("%d files found.\n",cnt);
  } // end if


  // turn on backlight
  pinMode(BACKLIGHT_LED, OUTPUT);
  digitalWrite(BACKLIGHT_LED, HIGH);
  glcd.begin(0);
  glcd.clear();

  for (int cont=0;cont<13;cont++) {
    glcd.st7565_set_brightness(cont);
  }
  glcd.st7565_set_brightness(12);

  int x = 0;
  for(int i=0;i<sizeof(str)/sizeof(str[0]);i++){
    getFontUTF(fx16, str[i], &utfinfo);
//    dumpFontUTF(&utfinfo);
    glcd.drawUTF(x, 0, utfinfo.font, utfinfo.wbit, utfinfo.hbit);
    x = x + utfinfo.wbit;
  }

  x = 0;
  for(int i=0;i<sizeof(str)/sizeof(str[0]);i++){
    getFontUTF(fx24, str[i], &utfinfo);
//    dumpFontUTF(&utfinfo);
    glcd.drawUTF(x, 2, utfinfo.font, utfinfo.wbit, utfinfo.hbit);
    x = x + utfinfo.wbit;
  }
  glcd.display();
  delay(5000);

  glcd.clear();
  x = 0;
  for(int i=0;i<sizeof(str)/sizeof(str[0]);i++){
    getFontUTF(fx32, str[i], &utfinfo);
//    dumpFontUTF(&utfinfo);
    glcd.drawUTF(x, 0, utfinfo.font, utfinfo.wbit, utfinfo.hbit);
    x = x + utfinfo.wbit;
  }
  glcd.display();
  delay(5000);
}

void loop()                     
{}


