// Teensy capacitance meter
// measures 100p-100n

#include <inttypes.h>
#include "config.h"
#include "LC_Math.h"
#include "7_Display.h"

IntervalTimer pwmTimer;

uint8_t measuredTimeDelay_int = 0;

bool doRotate = false;

elapsedMillis elapsedtimeSinceDetection;
elapsedMillis elapsedDigitTime;
elapsedMillis resetTriesTime;
elapsedMillis rotateResistanceTime;

void displayDigit();
void makePWM();
void displayInterrupt();
void rotateResistance();

uint8_t display_number = 0;

float medTimeDelay = 0;
float capacitance = 0;
uint8_t capacitance_exponent = 0;
void displayDigit() {

  if(LC_Math::measurementReady() && LC_Math::isCapacitorPresent()) {
    elapsedtimeSinceDetection = 0;
    LC_Math::getLastestMeasurement(capacitance, capacitance_exponent);
    
    display_number++;
    display_number = display_number % 3;
    switch(display_number) {
      case 0:
        SegmentDisplay::displayLogic(true, (uint8_t)(capacitance / 100.0));
        break;
      case 1:
        SegmentDisplay::displayLogic(true, (uint8_t)(capacitance));
        break;
      case 2:
        SegmentDisplay::displayLogic(true, capacitance_exponent);
        LC_Math::measurementHasBeenShown();
        break;
    }
  } else {
    SegmentDisplay::displayLogic(false, 42);
  }
  


}

bool pwmOut = false;
void makePWM() {

  pwmOut = !pwmOut;

  // TODO:
  // Possible time error between digitalWrite
  // and setting the timer to 0?
  digitalWrite(PULSE_PIN, pwmOut);
  LC_Math::resetTimers();

}

void setup() {

  LC_Math::init();
  SegmentDisplay::init();
  
  pinMode(PULSE_PIN, OUTPUT);

  pwmTimer.begin(makePWM, PULSE_ON_TIME);

}

#define MAX_TRIES 12
uint8_t rotateTries = 0;
void rotateResistance() {
    if(rotateTries < MAX_TRIES) {
      LC_Math::changeResistor();
    }
    
    rotateTries++;
    doRotate = false;
}

void loop() {
  LC_Math::update();
  if (elapsedtimeSinceDetection > 5000) {
    doRotate = true;
    rotateResistance();
    elapsedtimeSinceDetection = elapsedtimeSinceDetection - 5000;
  }

  if(elapsedDigitTime > 500) {
    elapsedDigitTime = elapsedDigitTime - 500;
    displayDigit();
  }

  if(resetTriesTime > 5000) {
    resetTriesTime = resetTriesTime - 5000;
    rotateTries = 0;
  }

}
