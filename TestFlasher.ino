#include "Flasher.h"

// create flashers to control 6 LEDs simultaneously and independently
// (one flasher controls the built-in LED, four flashers simulate a binary counter, one flasher demonstrates a more complicated flash pattern)
unsigned long warningFlash[] = {200, 200, 200, 200, 200, 1000}; // a 2-second warning pattern, three flashes in one second followed by a one second pause
Flasher flash1(LED_BUILTIN, warningFlash, sizeof(warningFlash)/sizeof(unsigned long));


Flasher flash2(2, 2000, 0, false);  // 2-seconds on, 2-seconds off
Flasher flash3(3, 4000, 0, false);  // 4-seconds on, 4-seconds off
Flasher flash4(4, 8000, 0, false);  // 8-seconds on, 8-seconds off
Flasher flash5(5, 16000, 0, false); // 16-seconds on, 16-seconds off

unsigned long accelFlash[] = {10000, 1000, 9000, 900, 8000, 800, 7000, 700, 6000, 600, 5000, 500, 4000, 400, 3000, 300, 2000, 200, 1000, 100, 1024, 1024, 512, 512, 256, 256, 128, 128, 64, 64, 32, 32, 16, 16, 8, 8, 4, 4, 2, 2, 1, 1};
Flasher flash6(6, accelFlash, sizeof(accelFlash)/sizeof(unsigned long));  // long pattern of accelerating flashes

// table of flags for testing actions (only 2 in use, so far)
unsigned long testTime;
bool doAction[] = {true, true, true, true, true, true};

void setup() {
  // put your setup code here, to run once:
  testTime = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  Flasher::loop();
  unsigned long t = millis();

  // testing the pause/resume functionality
  if (t > testTime + 5250) {
    if (doAction[0]) {
      flash2.pause();
      doAction[0] = false;
    }
  }
  if (t > testTime + 10000) {
    if (doAction[1]) {
      flash2.resume();
      doAction[1] = false;
    }
  }
}
