#include "LC_Math.h"
#include "config.h"


uint8_t LC_Math::currentRes = 1;
elapsedMicros LC_Math::elapsedtime_US = 0;
elapsedMillis LC_Math::elapsedtime_MS = 0;

uint16_t LC_Math::_measurementsBuffer[NUM_MEASUREMENTS] = {0};
uint16_t LC_Math::_lastMeasurement = 0;
uint8_t LC_Math::_i_Measurement = 0;

void LC_Math::init() {
	pinMode(RESISTOR_SELECT_PIN, OUTPUT);
	pinMode(DELAY_INPUT_PIN, INPUT);

	digitalWrite(RESISTOR_SELECT_PIN, currentRes % 2);

	attachInterrupt(DELAY_INPUT_PIN, _measureDelay, FALLING);
}

void LC_Math::changeResistor() {
	currentRes++;
	currentRes = currentRes % 2;
	digitalWrite(RESISTOR_SELECT_PIN, currentRes % 2);
}

void LC_Math::resetTimers() {
	elapsedtime_US = 0;
	elapsedtime_MS = 0;
}

bool LC_Math::measurementComplete() {
	return _i_Measurement >= NUM_MEASUREMENTS;
}

uint16_t LC_Math::getCurrentMeasurement() {
	return _i_Measurement;
}


// Formula is C = t / (R * Ln(Vsupply/Vthreshold))
// it can be optimized by making this constant
// log_ratio_volt = 1/Ln(Vsupply/Vthreshold)
// and because R is always an exponent of 10 (R=10k or 100k)
// the exponent of the scientific notation can be removed from the
// time which is always measured in microseconds, so 10^(-6) / 10^n
// 10^(-6-n)
// where n is currentRes+4 (because of our choise of resistors
// 10^(-10-currentRes)
// Equals to:
// C = log_ratio_volt*t*10^(-10-currentRes) in Farads
float LC_Math::calculateFastCapacitance(float median_time, uint8_t& exponent) {
  exponent = 10 + currentRes;
  return log_ratio_volt*median_time;
}

// This function does the same thing as the one above but takes into
// account the REAL resistances values.
// C = 1/(R)log_ratio_volt*t*10^(-6) in Farads
// Much slower, but more precise.
float LC_Math::calculatePreciseCapacitance(float median_time) {
  return (log_ratio_volt*median_time*10e-6 / resistancesArrays[currentRes]);
}

float LC_Math::calculateMedian() {
	float __medTimeDelay = 0;
	for(int i = 0; i < NUM_MEASUREMENTS; i++) {
        __medTimeDelay += _measurementsBuffer[i];
        _measurementsBuffer[i] = 0;
    }
    
    __medTimeDelay /= NUM_MEASUREMENTS;
    return __medTimeDelay;
}


void LC_Math::_measureDelay(void) {

  _lastMeasurement = elapsedtime_US;

  if(elapsedtime_MS > 0) {
    _lastMeasurement += 1000*( elapsedtime_MS - 1 );
  }

  //We decrease one because thats the time it takes for the interrupt
  //to execute from detection
  _lastMeasurement--;
  
  if(_i_Measurement < NUM_MEASUREMENTS) {
        
      if (_lastMeasurement > 5 || _lastMeasurement < MEASUREMENT_MAX_TIME) {
        _measurementsBuffer[_i_Measurement] = _lastMeasurement;
        _i_Measurement++;
      }
     
  }

  //elapsedtimeSinceDetection = 0;

}