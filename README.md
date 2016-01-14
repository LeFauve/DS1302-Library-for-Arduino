# DS1302-Library-for-Arduino
A simple library for driving the DS1302 RTC chip from an arduino.

This is basically the arduino.cc sample turned into a library for easier reusability.
It's not perfect but it does the job.

Original version by arduino.cc user "Krodal".
See this page for more details: http://playground.arduino.cc/Main/DS1302

Installation:
- Copy the DS1302 folder in your Arduino/libraries folder
- Restart your Arduino IDE

Basic usage:
First, you need to create a DS1302 instance with the pin used as parameters:
	DS1302(byte dataPin, byte clockPin, byte strobePin);

Then, you may call the following methods:

For setting the clock:
	void clockWrite(byte year, byte month, byte day, byte wday, byte hours, byte minutes, byte seconds);
	void clockWrite(DS1302_clock* c);
	void clockWrite12(DS1302_clock* c);

For reading the clock:
	DS1302_clock* clockRead(); // Read the clock in burst mode and return it in a ready to use object
	DS1302_clock* clockRead12(); // Read the clock in burst mode and return it in a ready to use 12h mode

The DS1302_clock class has the following attributes:
	byte year: Year (0-99; correspond to 2000-2099 (for correct bisextil years handling))
	byte month; Month (1-12)
	byte day; Day of month(1-31)
	byte wday; Day of week (1-7; note that you can chose any day you want as week start)
	byte pm; am/pm (only used with clockRead12() and clockWrite12() methods)
	byte hours; (0-24 or 1-12)
	byte minutes; (0-59)
	byte seconds; (0-59)

Note that clockRead12() and clockWrite12() are for 12 hours mode support; while the DS1302 support native 12h mode, this library always uses it in 24h mode and perform the translation.
This allows you to display both 12h and 24h time if you ever want to (without having to reprogram the DS1302).

You may look into the sketch examples for usage guideline, and in the DS1302.h file for complete list of available methods.
