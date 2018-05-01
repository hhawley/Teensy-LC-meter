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
#define MEASUREMENT_MAX_TIME PULSE_ON_TIME/2 //us
#define MEASUREMENT_MIN_TIME 25 //us

/// Change this values to the measurements you made
/// for more precise measurements.
#define TOTAL_RES 2
#define R1_M 9.93e3 // Ohms
#define R2_M 98.2e3 // Ohms
#define V_THRESHOLD 1.240 //V
#define V_SUPPLY 3.27 //V
///

#define log_ratio_volt 1.0/log(V_SUPPLY/V_THRESHOLD)

const float resistancesArrays[TOTAL_RES] = {R1_M, R2_M};

class LC_Math {
public:

	static uint8_t currentRes;
	static elapsedMicros elapsedtime_US;
	static elapsedMillis elapsedtime_MS;

	static void init();
	static void changeResistor();
	static float calculateFastCapacitance(float median_time, uint8_t& exponent);
	static float calculatePreciseCapacitance(float median_time);

	static void resetTimers();
	static bool measurementComplete();
	static float calculateMedian();

private:
	static void _measureDelay();

	static uint16_t _measurementsBuffer[NUM_MEASUREMENTS];
	static uint16_t _lastMeasurement;
	static uint8_t _i_Measurement;

};