/*
 *  ETW-11256K1にANK文字を表示するサンプル 
 *  フォントは5*8ドット
 */
#include <NT7502.h> // https://github.com/nopnop2002/NT7502-LCD
//#include <ST7565.h> // https://github.com/adafruit/ST7565-LCD

// the LCD backlight is connected up to a pin so you can turn it on & off
#define BACKLIGHT_LED 4

ST7565 glcd(11, 13, 7, 6, 5);
//ST7565 glcd(5, 14, 12, 13, 15);

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
    glcd.drawchar(0, 0, 1, cont1+48); // X1
    glcd.drawchar(5, 0, 1, cont2+48); // X1
    glcd.drawchar(10, 1, 2, cont1+48); // X2
    glcd.drawchar(20, 1, 2, cont2+48); // X2
    glcd.drawchar(20, 3, 3, cont1+48); // X2
    glcd.drawchar(35, 3, 3, cont2+48); // X2
    glcd.display();
    delay(1000);
  }

  glcd.st7565_set_brightness(5);
  glcd.clear();
  
  for(int i=48;i<58;i++) {
    glcd.drawchar((i-48)*6, 0, 1, i);
  }
  for(int i=48;i<57;i++) {
    glcd.drawchar((i-48)*12, 1, 2, i);
  }
  for(int i=48;i<53;i++) {
    glcd.drawchar((i-48)*18, 3, 3, i);
  }
  glcd.display();
  delay(4000);

  glcd.clear();
  for(int i=65;i<83;i++) {
    glcd.drawchar((i-65)*6, 0, 1, i);
  }
  for(int i=65;i<74;i++) {
    glcd.drawchar((i-65)*12, 1, 2, i);
  }
  for(int i=65;i<70;i++) {
    glcd.drawchar((i-65)*18, 3, 3, i);
  }
  glcd.display();

}

void loop()                     
{}


