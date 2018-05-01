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
  if(LC_Math::measurementComplete()) {
    elapsedtimeSinceDetection = 0;
    medTimeDelay = LC_Math::calculateMedian();

    if (MEASUREMENT_MIN_TIME < 25 || medTimeDelay > MEASUREMENT_MAX_TIME) {
        doRotate = true;
        rotateResistance();
        medTimeDelay = 0;
    }
    capacitance = LC_Math::calculateFastCapacitance(medTimeDelay, capacitance_exponent);
  }

  display_number++;
  display_number = display_number % 4;
  if (display_number == 0) {
    SegmentDisplay::displayLogic(true, (uint8_t)(capacitance / 100));
  } 
  else if (display_number == 1) {
    SegmentDisplay::displayLogic(true, (uint8_t)capacitance);
  }
  else if (display_number == 2) {
    SegmentDisplay::displayLogic(true, capacitance_exponent);
  } 
  else if(display_number == 3){
    SegmentDisplay::displayLogic(false, 42);
  } 
  else {
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
  if (elapsedtimeSinceDetection > 500) {
    doRotate = true;
    rotateResistance();
    elapsedtimeSinceDetection = elapsedtimeSinceDetection - 500;
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
