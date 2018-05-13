/*
	Author: Komatura
	Description: Deals with the math regarding measurements and calculations
	of the capacitances and (WIP) inductances.

	TODO: add inductance meter math

*/

#pragma once

#include <inttypes.h>
#include <math.h>
#include <Arduino.h>

// Measurement Defines
#define NUM_MEASUREMENTS 64

// Math Defines

#define PULSE_ON_TIME 10000 //us
#define PULSE_OFF_TIME 10000 //us

#define US_TO_CLKCNTS 48
#define CLKCNTS_TO_US 1/48.0
#define MEASUREMENT_MAX_TIME (PULSE_ON_TIME/2)*US_TO_CLKCNTS //clk ctns
#define MEASUREMENT_MIN_TIME 25*US_TO_CLKCNTS //clk ctns

/// Change this values to the measurements you made
/// for more precise measurements.
#define TOTAL_RES 2
#define R1_M 9.93e3 // Ohms
#define R2_M 98.2e3 // Ohms
#define V_THRESHOLD 1.240 //V
#define V_SUPPLY 3.33 //V

#define TOTAL_CAP 1
#define C1_M 100e-9
///

#define log_ratio_volt 1.0/log(V_SUPPLY/V_THRESHOLD)

const float resistancesArrays[TOTAL_RES] = {R1_M, R2_M};

class LC_Math {
public:

	static void init();
	static void update();
	static bool measurementReady();

	static void changeResistor();

	static void getLastestMeasurement(float& capacitance, uint8_t& exponent);
	static bool measurementsComplete();
	static void measurementHasBeenShown();
	static bool isCapacitorPresent();

	static void resetTimers();


private:
	static void _measureDelay();
	static void _measureFreq();
	static float _calculateFastCapacitance(float median_time, uint8_t& exponent);
	static float _calculatePreciseCapacitance(float median_time);
	static float _calculatePreciseInductance(float median_time);
	
	static float _calculateMedian();

	static float _measuredCapacitance;
	static uint8_t _measuredCapacitanceExponent;

	static uint32_t _measurementsBuffer[NUM_MEASUREMENTS];
	static uint32_t _lastMeasurement;
	static uint32_t _previousMeasurement;
	static uint32_t _elapsedtime_CS;
	static uint8_t _i_Measurement;

	static uint8_t _currentRes;
	static elapsedMillis _elapsedtime_MS;
	static elapsedMillis _elapsedtimeSinceDetection;

	static bool _measurementShown;

};
