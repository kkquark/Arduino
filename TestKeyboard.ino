#include "Keyboard.h"
#include "LCD.h"

unsigned int kbOutPins[] = {2, 3, 4, 5};
unsigned int kbInPins[] = {6, 7, 8, 12};
int kbNoOutPins = sizeof(kbOutPins) / sizeof(unsigned int);
int kbNoInPins = sizeof(kbInPins) / sizeof(unsigned int);
Keyboard kb(kbNoOutPins, kbOutPins, kbNoInPins, kbInPins, "0123456789SREDTA", 50, 300, 100, 'A', "abcdefghijklmnoA", 13);

// LDC stuff is for next project
#define LCD_RS      A5
#define LCD_ENABLE  A4
#define LCD_D4      A3
#define LCD_D5      A2
#define LCD_D6      A1
#define LCD_D7      A0

unsigned int lcdPins[] = {LCD_RS, LCD_ENABLE, LCD_D4, LCD_D5, LCD_D6, LCD_D7};
LiquidCrystal lcd(lcdPins[0], lcdPins[1], lcdPins[2], lcdPins[3], lcdPins[4], lcdPins[5]);

#define RED   10
#define GREEN 11
#define BLUE  9
long t;
long count;
int red = 0;
int green = 0;
int blue = 0;

void setup() {
  Serial.begin(9600);
  t = millis();
  count = 0;
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  lcd.begin(16, 4);
}

void loop() {
  // put your main code here, to run repeatedly:
  // required call to monitor keyboard
  kb.loop();
  // look for a key
  char key = kb.getKey();
  if (key) {
    Serial.print(key);
    // this code lets the brightness of an RGB LED be adjusted from the keyboard
    // first column of keys (0|4|8|E) adjusts red part of RGB LED (0: off, 4: step down, 8: step up, E: max)
    // second column (1|5|9|D) adjusts green part of LED
    // third column (2|6|S|T) adjusts blue part of LED
    // forth column (3|7|R) adjusts R/G/B together
    // final key in forth column (A) is designated as the shift key, and makes the step up and down keys use smaller steps
    //      so unshifted key for coarse level setting shifted for fine level setting (hold shift key down while pressing other keys)
    switch (key) {
      case '0':
      case 'a':
        red = 0;
        analogWrite(RED, red);
        break;
      case '4':
        red = max(0, red - 5);
        analogWrite(RED, red);
        break;
      case 'e':
        red = max(0, red - 1);
        analogWrite(RED, red);
        break;
      case '8':
        red = min(255, red + 5);
        analogWrite(RED, red);
        break;
      case 'i':
        red = min(255, red + 1);
        analogWrite(RED, red);
        break;
      case 'E':
      case 'm':
        red = 255;
        analogWrite(RED, red);
        break;
      case '1':
      case 'b':
        green = 0;
        analogWrite(GREEN, green);
        break;
      case '5':
        green = max(0, green - 5);
        analogWrite(GREEN, green);
        break;
      case 'f':
        green = max(0, green - 1);
        analogWrite(GREEN, green);
        break;
      case '9':
        green = min(255, green + 5);
        analogWrite(GREEN, green);
        break;
      case 'j':
        green = min(255, green + 1);
        analogWrite(GREEN, green);
        break;
      case 'D':
      case 'n':
        green = 255;
        analogWrite(GREEN, green);
        break;
      case '2':
      case 'c':
        blue = 0;
        analogWrite(BLUE, blue);
        break;
      case '6':
        blue = max(0, blue - 5);
        analogWrite(BLUE, blue);
        break;
      case 'g':
        blue = max(0, blue - 1);
        analogWrite(BLUE, blue);
        break;
      case 'S':
        blue = min(255, blue + 5);
        analogWrite(BLUE, blue);
        break;
      case 'k':
        blue = min(255, blue + 1);
        analogWrite(BLUE, blue);
        break;
      case 'T':
      case 'o':
        blue = 255;
        analogWrite(BLUE, blue);
        break;
      case '3':
      case 'd':
        red = green = blue = 0;
        analogWrite(RED, red);
        analogWrite(GREEN, green);
        analogWrite(BLUE, blue);
        break;
      case '7':
        red = max(0, red - 5);
        green = max(0, green - 5);
        blue = max(0, blue - 5);
        analogWrite(RED, red);
        analogWrite(GREEN, green);
        analogWrite(BLUE, blue);
        break;
      case 'h':
        red = max(0, red - 1);
        green = max(0, green - 1);
        blue = max(0, blue - 1);
        analogWrite(RED, red);
        analogWrite(GREEN, green);
        analogWrite(BLUE, blue);
        break;
      case 'R':
        red = min(255, red + 5);
        green = min(255, green + 5);
        blue = min(255, blue + 5);
        analogWrite(RED, red);
        analogWrite(GREEN, green);
        analogWrite(BLUE, blue);
        break;
      case 'l':
        red = min(255, red + 1);
        green = min(255, green + 1);
        blue = min(255, blue + 1);
        analogWrite(RED, red);
        analogWrite(GREEN, green);
        analogWrite(BLUE, blue);
        break;
    }
  }
  count++;
}
