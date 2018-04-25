// Teensy capacitance meter
// measures 100p-100n

#include <inttypes.h>

// Pin Defines
#define A 0
#define B 1
#define SEGMENT_1 2
#define SEGMENT_2 3
#define SEGMENT_BIT_0 7
#define SEGMENT_BIT_1 5
#define SEGMENT_BIT_2 4
#define SEGMENT_BIT_3 6
#define DELAY_INPUT_PIN 20 
#define PULSE_PIN 21

//Math Defines
#define PULSE_ON_TIME 10000 //us
#define PULSE_OFF_TIME 10000 //us
#define MEASUREMENT_MAX_TIME PULSE_ON_TIME/2 //us
#define MEASUREMENT_MIN_TIME 25 //us
#define V_THRESHOLD 1.240 //V
#define V_SUPPLY 3.27 //V
#define log_ratio_volt 1/log(V_SUPPLY/V_THRESHOLD)

IntervalTimer pwmTimer;
IntervalTimer displayTimer;

uint8_t measuredTimeDelay_int = 0;
uint8_t displayNum_int = 42;
uint8_t displayValue_int = 42;

#define NUM_MEASUREMENTS 64
uint8_t currentRes = 1;
uint16_t measurementsBuffer[NUM_MEASUREMENTS] = {0};
uint16_t lastMeasurement = 0;
uint8_t i_Measurement = 0;
bool doRotate = false;

bool displayState = false;

elapsedMicros elapsedtimeUSSinceFalling;
elapsedMillis elapsedtimeMSSinceFalling;
elapsedMillis elapsedtimeSinceDetection;
elapsedMillis elapsedDigitTime;
elapsedMillis resetTriesTime;
elapsedMillis rotateResistanceTime;

void displayDigit();
void makePWM();
void displayInterrupt();
void rotateResistance();

uint8_t display_number = 0;
// Formula is C = t / (R * Ln(Vsupply/Vthreshold))
// it can be optimized by making constant
// log_ratio_volt = 1/Ln(Vsupply/Vthreshold)
// and because R is always an exponent of 10 (R=10k or 100k)
// the exponent of the scientific notation can be removed from the
// time which is always measured in microseconds, so 10^(-6) / 10^n
// 10^(-6-n)
// where n is currentRes+4 (because of our choise of resistors
// 10^(-10-currentRes)
// Equals to:
// C = log_ratio_volt*t*10^(-10-currentRes) in Farads
float calculateCapacitance(float median_time, uint8_t& exponent) {
  exponent = 10+currentRes;
  return log_ratio_volt*median_time;
}

float medTimeDelay = 0;
float capacitance = 0;
uint8_t capacitance_exponent = 0;
void displayDigit() {
  if(i_Measurement >= NUM_MEASUREMENTS) {
    medTimeDelay = 0;

    for(int i = 0; i < NUM_MEASUREMENTS; i++) {
        medTimeDelay += measurementsBuffer[i];
        measurementsBuffer[i] = 0;
    }
    
    medTimeDelay /= NUM_MEASUREMENTS;
    if (MEASUREMENT_MIN_TIME < 25 || medTimeDelay > MEASUREMENT_MAX_TIME) {
        doRotate = true;
        rotateResistance();
        medTimeDelay = 0;
    }

    capacitance = calculateCapacitance(medTimeDelay, capacitance_exponent);

    i_Measurement = 0;
  }

  display_number++;
  display_number = display_number % 4;
  displayValue_int = 0;
  if (display_number == 0) {
    displayValue_int = (uint16_t)(capacitance / 100);
    displayState = true;
  } 
  else if (display_number == 1) {
    displayValue_int = (uint16_t)capacitance;
    displayState = true;
  }
  else if (display_number == 2) {
    displayValue_int = capacitance_exponent;
    displayState = true;
  } else if(display_number == 3){
    displayValue_int = 0;
    displayState = false;
  } else {
    displayValue_int = 1;
  }

}

bool pwmOut = false;
void makePWM() {

  pwmOut = !pwmOut;

  // TODO:
  // Possible time error between digitalWrite
  // and setting the timer to 0?
  digitalWrite(PULSE_PIN, pwmOut);
  elapsedtimeUSSinceFalling = 0;
  elapsedtimeMSSinceFalling = 0;

}

uint8_t displaySegment = 0;
void displayInterrupt() {

  digitalWrite(SEGMENT_1, LOW);
  digitalWrite(SEGMENT_2, LOW);

  if(!displayState) { return; }

  displayNum_int = displayValue_int;
  if ((displaySegment % 2) == 0) {

    int digit1 = displayNum_int % 10;
    digitalWrite(SEGMENT_BIT_0, (1 & (digit1 >> 0)));
    digitalWrite(SEGMENT_BIT_1, 1 & (digit1 >> 1));
    digitalWrite(SEGMENT_BIT_2, 1 & (digit1 >> 2));
    digitalWrite(SEGMENT_BIT_3, 1 & (digit1 >> 3));
    digitalWrite(SEGMENT_1, HIGH);

  } else  {

    int digit2 = (displayNum_int / 10) % 10;
    digitalWrite(SEGMENT_BIT_0, (1 & (digit2 >> 0)));
    digitalWrite(SEGMENT_BIT_1, 1 & (digit2 >> 1));
    digitalWrite(SEGMENT_BIT_2, 1 & (digit2 >> 2));
    digitalWrite(SEGMENT_BIT_3, 1 & (digit2 >> 3));
    digitalWrite(SEGMENT_2, HIGH);

  }

  displaySegment++;

}



void setup() {
  pinMode(A, OUTPUT);
  //pinMode(B, OUTPUT);
  pinMode(SEGMENT_1, OUTPUT);
  pinMode(SEGMENT_2, OUTPUT);
  pinMode(SEGMENT_BIT_0, OUTPUT);
  pinMode(SEGMENT_BIT_1, OUTPUT);
  pinMode(SEGMENT_BIT_2, OUTPUT);
  pinMode(SEGMENT_BIT_3, OUTPUT);
  pinMode(PULSE_PIN, OUTPUT);
  pinMode(DELAY_INPUT_PIN, INPUT);

  digitalWrite(SEGMENT_1, LOW);
  digitalWrite(SEGMENT_2, LOW);
  digitalWrite(SEGMENT_BIT_0, LOW);
  digitalWrite(SEGMENT_BIT_1, LOW);
  digitalWrite(SEGMENT_BIT_2, LOW);
  digitalWrite(SEGMENT_BIT_3, LOW);
  digitalWrite(A, currentRes % 2);
  //digitalWrite(B, b_values[currentRes % 4]);

  pwmTimer.begin(makePWM, PULSE_ON_TIME);
  displayTimer.begin(displayInterrupt, 10000);
  
  attachInterrupt(DELAY_INPUT_PIN, measureDelay, FALLING);

}

#define MAX_TRIES 12
uint8_t rotateTries = 0;
void rotateResistance() {


    if(rotateTries < MAX_TRIES) {
        currentRes++;
        currentRes = currentRes % 2;
        digitalWrite(A, currentRes);
    }
    
    rotateTries++;
    doRotate = false;
  
}

void measureDelay(void) {

  lastMeasurement = elapsedtimeUSSinceFalling;

  if(elapsedtimeMSSinceFalling > 0) {
    lastMeasurement += 1000*(elapsedtimeMSSinceFalling-1);
  }
  //We decrease one because thats the time it takes for the interrupt
  //to execute from detection
  lastMeasurement--;
  
  if(i_Measurement < NUM_MEASUREMENTS) {
        
      if (lastMeasurement > 5 || lastMeasurement < MEASUREMENT_MAX_TIME) {
        measurementsBuffer[i_Measurement] = lastMeasurement;
        i_Measurement++;
      }
     
  }

  elapsedtimeSinceDetection = 0;

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
