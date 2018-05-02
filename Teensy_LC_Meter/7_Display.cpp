#include "7_Display.h"
#include "config.h"

bool SegmentDisplay::_displayState = false;
uint8_t SegmentDisplay::displayNum_int = 42;
uint8_t SegmentDisplay::_displaySegment = 0;
IntervalTimer SegmentDisplay::_displayTimer;

void SegmentDisplay::init() {

	pinMode(SEGMENT_1, OUTPUT);
	pinMode(SEGMENT_2, OUTPUT);
	pinMode(SEGMENT_BIT_0, OUTPUT);
	pinMode(SEGMENT_BIT_1, OUTPUT);
	pinMode(SEGMENT_BIT_2, OUTPUT);
	pinMode(SEGMENT_BIT_3, OUTPUT);

	digitalWrite(SEGMENT_1, LOW);
	digitalWrite(SEGMENT_2, LOW);
	digitalWrite(SEGMENT_BIT_0, LOW);
	digitalWrite(SEGMENT_BIT_1, LOW);
	digitalWrite(SEGMENT_BIT_2, LOW);
	digitalWrite(SEGMENT_BIT_3, LOW);

	_displayTimer.begin(_displayInterrupt, DISPLAY_TIMER);

}

void SegmentDisplay::displayLogic(const bool& doDisplay, const uint8_t& digitToShow) {
	displayNum_int = digitToShow;
	_displayState = doDisplay;
}

void SegmentDisplay::_displayInterrupt() {

  digitalWrite(SEGMENT_1, LOW);
  digitalWrite(SEGMENT_2, LOW);

  if(!_displayState) { return; }

  if ((_displaySegment % NUM_SEGMENTS) == 0) {

    int digit1 = displayNum_int % 10;
    digitalWrite(SEGMENT_BIT_0, 1 & (digit1 >> 0));
    digitalWrite(SEGMENT_BIT_1, 1 & (digit1 >> 1));
    digitalWrite(SEGMENT_BIT_2, 1 & (digit1 >> 2));
    digitalWrite(SEGMENT_BIT_3, 1 & (digit1 >> 3));
    digitalWrite(SEGMENT_1, HIGH);

  } else  {

    int digit2 = (displayNum_int / 10) % 10;
    digitalWrite(SEGMENT_BIT_0, 1 & (digit2 >> 0));
    digitalWrite(SEGMENT_BIT_1, 1 & (digit2 >> 1));
    digitalWrite(SEGMENT_BIT_2, 1 & (digit2 >> 2));
    digitalWrite(SEGMENT_BIT_3, 1 & (digit2 >> 3));
    digitalWrite(SEGMENT_2, HIGH);

  }

  _displaySegment++;

}
