// Library for using the DS1302 RTC
//
// This is basically the arduino.cc sample turned into a library for easier reusability.
// It's not perfect but it does the job.
//
// Original version by arduino.cc user "Krodal".
// See this page for more details: http://playground.arduino.cc/Main/DS1302

#include "Arduino.h"
#include "DS1302.h"

#define bcd2bin(h,l)    (((h)*10) + (l))
#define bin2bcd_h(x)   ((x)/10)
#define bin2bcd_l(x)    ((x)%10)

// Defines for the bits, to be able to change 
// between bit number and binary definition.
// By using the bit number, using the DS1302 
// is like programming an AVR microcontroller.
// But instead of using "(1<<X)", or "_BV(X)", 
// the Arduino "bit(X)" is used.                   
#define DS1302_D0 0
#define DS1302_D1 1
#define DS1302_D2 2
#define DS1302_D3 3
#define DS1302_D4 4
#define DS1302_D5 5
#define DS1302_D6 6
#define DS1302_D7 7


// Bit for reading (bit in address)
#define DS1302_READBIT DS1302_D0 // READBIT=1: read instruction

// Bit for clock (0) or ram (1) area, called R/C-bit (bit in address)
#define DS1302_RC DS1302_D6

                   
// Seconds Register
#define DS1302_CH DS1302_D7   // 1 = Clock Halt, 0 = start

// Hour Register
#define DS1302_AM_PM DS1302_D5 // 0 = AM, 1 = PM
#define DS1302_12_24 DS1302_D7 // 0 = 24 hour, 1 = 12 hour

// Enable Register
#define DS1302_WP DS1302_D7   // 1 = Write Protect, 0 = enabled

// Trickle Register
#define DS1302_ROUT0 DS1302_D0
#define DS1302_ROUT1 DS1302_D1
#define DS1302_DS0   DS1302_D2
#define DS1302_DS1   DS1302_D2 // There *may* be an error here (to check asap)
#define DS1302_TCS0  DS1302_D4
#define DS1302_TCS1  DS1302_D5
#define DS1302_TCS2  DS1302_D6
#define DS1302_TCS3  DS1302_D7            
         
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DS1302::DS1302(byte dataPin, byte clockPin, byte strobePin) {
	this->dataPin = dataPin;
	this->clockPin = clockPin;
	this->strobePin = strobePin;

  // Start by clearing the Write Protect bit otherwise the clock data cannot be written
  // The whole register is written, but the WP-bit is the only bit in that register.
  registerWrite (DS1302_ENABLE, 0);

  // Disable Trickle Charger.
  registerWrite (DS1302_TRICKLE, 0x00);       
}

DS1302_clock* DS1302::clockRead() {
	DS1302_struct dt;
	static DS1302_clock theClock;
	clockBurstRead(&dt);

	theClock.year = bcd2bin(dt.Year10, dt.Year);
	theClock.month = bcd2bin(dt.Month10, dt.Month);
	theClock.day = bcd2bin(dt.Date10, dt.Date);
	theClock.wday = dt.Day;
	theClock.hours = bcd2bin(dt.h24.Hour10, dt.h24.Hour);
    theClock.minutes = bcd2bin(dt.Minutes10, dt.Minutes);
    theClock.seconds = bcd2bin(dt.Seconds10, dt.Seconds);

	return &theClock;
}

DS1302_clock* DS1302::clockRead12() {
	DS1302_clock* pClock = this->clockRead();
	pClock->pm = pClock->hours >=12;
	if(pClock->hours>12) {
		pClock->hours -= 12;
	}
	else if(pClock->hours==0) {
		pClock->hours = 12;
	}

	return pClock;
}

// Set a time and date
// This also clears the CH (Clock Halt) bit, to start the clock.
void DS1302::clockWrite(DS1302_clock* c) {
	this->clockWrite(c->year, c->month, c->day, c->wday, c->hours, c->minutes, c->seconds);
}

void DS1302::clockWrite12(DS1302_clock* c) {
	this->clockWrite(c->year, c->month, c->day, c->wday, (c->hours%12)+(c->pm?12:0), c->minutes, c->seconds);
}

// Set a time and date
// This also clears the CH (Clock Halt) bit, to start the clock.
void DS1302::clockWrite(byte year, byte month, byte day, byte wday, byte hours, byte minutes, byte seconds) {
	DS1302_struct dt;
 	// Fill the structure with zeros to make 
 	// any unused bits zero
	*((long*)&dt) = *(((long*)&dt)+1) = 0; // 10 bytes shorter than memset ((char *) &dt, 0, sizeof(dt));

 	dt.Seconds    = bin2bcd_l(seconds);
 	dt.Seconds10  = bin2bcd_h(seconds);
 	dt.CH         = 0;                                    
 	dt.Minutes    = bin2bcd_l(minutes);
 	dt.Minutes10  = bin2bcd_h(minutes);
 	dt.h24.Hour   = bin2bcd_l(hours);
 	dt.h24.Hour10 = bin2bcd_h(hours);
 	dt.h24.hour_12_24 = 0; // 0 for 24 hour format
 	dt.Date       = bin2bcd_l(day);
 	dt.Date10     = bin2bcd_h(day);
 	dt.Month      = bin2bcd_l(month);
 	dt.Month10    = bin2bcd_h(month);
 	dt.Day        = wday;
 	dt.Year       = bin2bcd_l(year);
 	dt.Year10     = bin2bcd_h(year);
 	dt.WP = 0;

 	// Write all clock data at once (burst mode).                                             
 	clockBurstWrite(&dt);
}

// This function writes 8 bytes clock data in burst mode to the DS1302 from the shared buffer _DS1302.
// This function may be called as the first function, also the pinMode is set.
void DS1302::clockBurstWrite(DS1302_struct* buffer_out) {
  start();

  // Instead of the address, the CLOCK_BURST_WRITE command is issued.
  // the I/O-line is not released
  toggleWrite(DS1302_CLOCK_BURST_WRITE, false);  

  byte* p = (byte*)buffer_out;
  for(byte i=0; i<8; ++i) toggleWrite(*p++, false);  // the I/O-line is not released
  stop();
}

// This function reads 8 bytes clock data in burst mode from the DS1302 into the shared buffer _DS1302.
// This function may be called as the first function, also the pinMode is set.
void DS1302::clockBurstRead(DS1302_struct* buffer_in) {
  start();

  // Instead of the address, the CLOCK_BURST_READ command is issued
  // the I/O-line is released for the data
  toggleWrite(DS1302_CLOCK_BURST_READ, true);  

  byte* p = (byte*)buffer_in;
  for(byte i=0; i<8; ++i) *p++ = toggleRead();
  stop();
}

// A helper function to setup the start condition.
// An 'init' function is not used, but now the pinMode is set every time. That's not a big deal, and it's valid.
// At startup, the pins of the Arduino are high impedance.
// Since the DS1302 has pull-down resistors, the signals are low (inactive) until the DS1302 is used.
void DS1302::start() {
  digitalWrite(this->strobePin, LOW); // default, not enabled
  pinMode(this->strobePin, OUTPUT);  

  digitalWrite(this->clockPin, LOW); // default, clock low
  pinMode(this->clockPin, OUTPUT);

  pinMode(this->dataPin, OUTPUT);

  digitalWrite(this->strobePin, HIGH); // start the session
  delayMicroseconds(4); // tCC = 4us
}

// A helper function to finish the communication.
void DS1302::stop() {
  digitalWrite(this->strobePin, LOW); // Set CE low
  delayMicroseconds(4); // tCWH = 4us
}

// A helper function for reading a byte with bit toggle.
// This function assumes that the SCLK is still high.
byte DS1302::toggleRead() {
  byte data = 0;
  for(byte i = 0; i <= 7; ++i) {
    // Issue a clock pulse for the next databit.
    // If the 'togglewrite' function was used before this function, the SCLK is already high.
    digitalWrite(this->clockPin, HIGH);
    delayMicroseconds(1);

    digitalWrite(this->clockPin, LOW); // Clock down, data is ready after some time.
    delayMicroseconds(1); // tCL=1000ns, tCDD=800ns

    // read bit, and set it in place in 'data' variable
    bitWrite(data, i, digitalRead(this->dataPin)); 
  }
  return data;
}

// A helper function for writing a byte with bit toggle
// The 'release' parameter is for a read after this write. It will release the I/O-line and will keep the SCLK high.
void DS1302::toggleWrite(byte data, byte release) {
	for(byte i = 0; i <= 7; i++) { 
	    digitalWrite(this->dataPin, bitRead(data, i)); // set a bit of the data on the I/O-line
	    delayMicroseconds(1); // tDC = 200ns

	    digitalWrite(this->clockPin, HIGH); // clock up, data is read by DS1302
	    delayMicroseconds( 1); // tCH = 1000ns, tCDH = 800ns

	    if(release && i == 7) {
	      // If this write is followed by a read, the I/O-line should be released after the last bit, before the clock line is made low.
	      // This is according the datasheet. I have seen other programs that don't release the I/O-line at this moment,
	      // and that could cause a shortcut spike on the I/O-line.
	      pinMode(this->dataPin, INPUT);
	    }
	    else {
	      digitalWrite(this->clockPin, LOW);
	      delayMicroseconds( 1);       // tCL=1000ns, tCDD=800ns
	    }
	}
}

// This function reads a byte from the DS1302 (clock or ram).
// The address could be like "0x80" or "0x81", the lowest bit is set anyway.
// This function may be called as the first function, also the pinMode is set.
byte DS1302::registerRead(byte address) {
  bitSet(address, DS1302_READBIT);  // set lowest bit (read bit) in address

  start();
  toggleWrite(address, true);  // the I/O-line is released for the data
  byte data = toggleRead();
  stop();

  return data;
}

// This function writes a byte to the DS1302 (clock or ram).
// The address could be like "0x80" or "0x81", the lowest bit is cleared anyway.
// This function may be called as the first function, also the pinMode is set.
void DS1302::registerWrite(byte address, byte data) {
  bitClear(address, DS1302_READBIT); // clear lowest bit (read bit) in address
  start();
  toggleWrite(address, false); // don't release the I/O-line
  toggleWrite(data, false); // don't release the I/O-line
  stop();  
}
