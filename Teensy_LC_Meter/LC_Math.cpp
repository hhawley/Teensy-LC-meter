#include "LC_Math.h"
#include "config.h"

float LC_Math::_measuredCapacitance = 0.0f;
uint8_t LC_Math::_measuredCapacitanceExponent = 10;	

uint8_t LC_Math::_currentRes = 1;
elapsedMicros LC_Math::_elapsedtime_US = 0;
elapsedMillis LC_Math::_elapsedtime_MS = 0;
elapsedMillis LC_Math::_elapsedtimeSinceDetection = 0;

uint16_t LC_Math::_measurementsBuffer[NUM_MEASUREMENTS] = {0};
uint16_t LC_Math::_lastMeasurement = 0;
uint8_t LC_Math::_i_Measurement = 0;
bool LC_Math::_measurementShown = true;

void LC_Math::init() {
	pinMode(RESISTOR_SELECT_PIN, OUTPUT);
	pinMode(DELAY_INPUT_PIN, INPUT);

	digitalWrite(RESISTOR_SELECT_PIN, _currentRes % 2);

	attachInterrupt(DELAY_INPUT_PIN, _measureDelay, FALLING);
}

void LC_Math::update() {
	if(isCapacitorPresent() && _measurementShown) {
	  if(measurementsComplete()) {

  		_elapsedtimeSinceDetection = 0;
  		float medTimeDelay = LC_Math::_calculateMedian();
  
  	    if (medTimeDelay < MEASUREMENT_MIN_TIME || medTimeDelay > MEASUREMENT_MAX_TIME) {
  	        // doRotate = true;
  	        changeResistor();
  	        // medTimeDelay = 0;
  	    }
  
  		// _measuredCapacitanceExponent = 0;
  		// _measuredCapacitance = LC_Math::_calculateFastCapacitance(medTimeDelay, _measuredCapacitanceExponent);
  		
  
  		//  More precise measurement:
  		_measuredCapacitanceExponent = 8;
  		_measuredCapacitance = LC_Math::_calculatePreciseCapacitance(medTimeDelay);
  		_measuredCapacitance *= 1e8;
  		
  		while((uint16_t)(_measuredCapacitance) < 99) {
  			_measuredCapacitanceExponent++;
  			_measuredCapacitance *= 10;
  		} 
  
  		_measurementShown = false;

	  }
	}
}

bool LC_Math::measurementReady() {
	return !_measurementShown;
}

void LC_Math::getLastestMeasurement(float& capacitance, uint8_t& exponent) {
	capacitance = _measuredCapacitance;
	exponent = _measuredCapacitanceExponent;
}

void LC_Math::measurementHasBeenShown() {
	_measurementShown = true;

}

bool LC_Math::isCapacitorPresent() {
	return (_lastMeasurement > 8 && _lastMeasurement < MEASUREMENT_MAX_TIME);
}

void LC_Math::changeResistor() {
	_currentRes++;
	_currentRes = _currentRes % 2;
	digitalWrite(RESISTOR_SELECT_PIN, _currentRes % 2);

	  // Clean Measurements
	_i_Measurement = 0;
	for(int i = 0; i < NUM_MEASUREMENTS; i++) {
	    _measurementsBuffer[i] = 0;
	}
}

void LC_Math::resetTimers() {
	_elapsedtime_US = 0;
	_elapsedtime_MS = 0;
}

bool LC_Math::measurementsComplete() {
	return _i_Measurement >= NUM_MEASUREMENTS;
}


// Formula is C = t / (R * Ln(Vsupply/Vthreshold))
// it can be optimized by making this constant
// log_ratio_volt = 1/Ln(Vsupply/Vthreshold)
// and because R is always an exponent of 10 (R=10k or 100k)
// the exponent of the scientific notation can be removed from the
// time which is always measured in microseconds, so 10^(-6) / 10^n
// 10^(-6-n)
// where n is _currentRes+4 (because of our choise of resistors
// 10^(-10-_currentRes)
// Equals to:
// C = log_ratio_volt*t*10^(-10-_currentRes) in Farads
float LC_Math::_calculateFastCapacitance(float median_time, uint8_t& exponent) {
  exponent = 10 + _currentRes;
  return log_ratio_volt*median_time;
}

// This function does the same thing as the one above but takes into
// account the REAL resistances values.
// C = 1/(R)log_ratio_volt*t*10^(-6) in Farads
// Much slower, but more precise.
float LC_Math::_calculatePreciseCapacitance(float median_time) {
  return (log_ratio_volt*median_time*1e-6 / resistancesArrays[_currentRes]);
}

float LC_Math::_calculateMedian() {
	float __medTimeDelay = 0;
	for(int i = 0; i < NUM_MEASUREMENTS; i++) {
        __medTimeDelay += _measurementsBuffer[i];
        _measurementsBuffer[i] = 0;
    }
    
    __medTimeDelay /= NUM_MEASUREMENTS;
    _i_Measurement = 0;
    return __medTimeDelay;
}


void LC_Math::_measureDelay(void) {

  _lastMeasurement = _elapsedtime_US;

  if(_elapsedtime_MS > 0) {
    _lastMeasurement += 1000*( _elapsedtime_MS - 1 );
  }

  if(measurementsComplete() || !_measurementShown) { return; }

  //We decrease one because thats the time it takes for the interrupt
  //to execute from detection
  _lastMeasurement--;
  
  if(_i_Measurement < NUM_MEASUREMENTS) {
        
      if (_lastMeasurement > 5 || _lastMeasurement < MEASUREMENT_MAX_TIME) {
        _measurementsBuffer[_i_Measurement] = _lastMeasurement;
        _i_Measurement++;
      }
     
  }

  //_elapsedtimeSinceDetection = 0;

}
