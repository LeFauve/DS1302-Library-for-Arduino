// Library for using the DS1302 RTC
//
// This is basically the arduino.cc sample turned into a library for easier reusability.
// It's not perfect but it does the job.
//
// Original version by arduino.cc user "Krodal".
// See this page for more details: http://playground.arduino.cc/Main/DS1302

#ifndef __DS1302__
#define __DS1302__

#include "Arduino.h"
//#define DS1302_SAVE_VARS // Augment code size but reduce vars size (may be handy if you're very tight with memory)
struct DS1302_struct;

#ifdef DS1302_SAVE_VARS
class DS1302_clock {
public:
	byte year;
	byte month:4;
	byte day:5;
	byte wday:3;
	byte pm:1;
	byte hours:5;
	byte minutes:6;
	byte seconds;
};
#else
class DS1302_clock {
public:
	byte year;
	byte month;
	byte day;
	byte wday;
	byte pm;
	byte hours;
	byte minutes;
	byte seconds;
};
#endif

class DS1302 {
public:
	DS1302(byte dataPin, byte clockPin, byte strobePin);
	DS1302_clock* clockRead(); // Read the clock in burst mode and return it in a ready to use object
	DS1302_clock* clockRead12(); // Read the clock in burst mode and return it in a ready to use 12h mode
	void clockWrite(byte year, byte month, byte day, byte wday, byte hours, byte minutes, byte seconds); // Set the clock
	void clockWrite(DS1302_clock* c); // Other way of setting the clock 
	void clockWrite12(DS1302_clock* c); // Set the clock from a 12h formated object
	byte registerRead(byte address); // Read a register
	void registerWrite(byte address, byte data); // Write a register

	// You shouldn't need to use the following methods in normal usage but they may be convenient if you want low level
	// Communication with the DS1302 chip
	void start(); // Helper function to setup the start condition.
	void stop();  // Helper function to end the communication.
	byte toggleRead();
	void toggleWrite(byte data, byte release);
	void clockBurstWrite(DS1302_struct* buffer_in);
	void clockBurstRead(DS1302_struct* buffer_out);
protected:
	byte dataPin;
	byte clockPin;
	byte strobePin;
};

////////////////////////////////////////////////////////////////////////////////
// Register names: You only need them if you plan to call registerWrite() or
// registerRead() directly. 
// You may need them if you try to access the chip's RAM or the TRICKLE
// functions which are not yet supported by this library.
// Note: registerRead() will automatically set the lowest bit of the register
// address for you.
////////////////////////////////////////////////////////////////////////////////

#define DS1302_SECONDS           0x80
#define DS1302_MINUTES           0x82
#define DS1302_HOURS             0x84
#define DS1302_DATE              0x86
#define DS1302_MONTH             0x88
#define DS1302_DAY               0x8A
#define DS1302_YEAR              0x8C
#define DS1302_ENABLE            0x8E
#define DS1302_TRICKLE           0x90
#define DS1302_CLOCK_BURST       0xBE
#define DS1302_CLOCK_BURST_WRITE 0xBE
#define DS1302_CLOCK_BURST_READ  0xBF
#define DS1302_RAMSTART          0xC0
#define DS1302_RAMEND            0xFC
#define DS1302_RAM_BURST         0xFE
#define DS1302_RAM_BURST_WRITE   0xFE
#define DS1302_RAM_BURST_READ    0xFF

////////////////////////////////////////////////////////////////////////////////
// You probably don't need to use DS1302_struct, unless you're calling directly
// clockBurstRead() or clockBurstWrite, which you probably don't need to.
//
// Use clockWrite() and clockRead() instead.
////////////////////////////////////////////////////////////////////////////////

// Structure for the first 8 registers.
// These 8 bytes can be read at once with the 'clock burst' command.
// Note that this structure contains an anonymous union. It might cause a problem on other compilers.
typedef struct DS1302_struct {
  uint8_t Seconds:4;      // low decimal digit 0-9
  uint8_t Seconds10:3;    // high decimal digit 0-5
  uint8_t CH:1;           // CH = Clock Halt
  uint8_t Minutes:4;
  uint8_t Minutes10:3;
  uint8_t reserved1:1;
  union {
    struct {
      uint8_t Hour:4;
      uint8_t Hour10:2;
      uint8_t reserved2:1;
      uint8_t hour_12_24:1; // 0 for 24 hour format
    } h24;
    struct {
      uint8_t Hour:4;
      uint8_t Hour10:1;
      uint8_t AM_PM:1;      // 0 for AM, 1 for PM
      uint8_t reserved2:1;
      uint8_t hour_12_24:1; // 1 for 12 hour format
    } h12;
  };
  uint8_t Date:4;           // Day of month, 1 = first day
  uint8_t Date10:2;
  uint8_t reserved3:2;
  uint8_t Month:4;          // Month, 1 = January
  uint8_t Month10:1;
  uint8_t reserved4:3;
  uint8_t Day:3;            // Day of week, 1 = first day (any day)
  uint8_t reserved5:5;
  uint8_t Year:4;           // Year, 0 = year 2000
  uint8_t Year10:4;
  uint8_t reserved6:7;
  uint8_t WP:1;             // WP = Write Protect
} DS1302_struct;  


#endif