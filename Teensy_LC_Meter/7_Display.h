#pragma once

#include <inttypes.h>
#include <Arduino.h>

#define NUM_SEGMENTS 2
#define DISPLAY_TIMER 10000 //us

class SegmentDisplay {
public:
	static uint8_t displayNum_int;

	static void init();
	static void displayLogic(const bool&, const uint8_t&);

private:
	static IntervalTimer _displayTimer;
	static uint8_t _displaySegment;
	static bool _displayState;

	static void _displayInterrupt();
};