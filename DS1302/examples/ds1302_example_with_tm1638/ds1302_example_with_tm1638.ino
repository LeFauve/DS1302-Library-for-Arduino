#include <DS1302.h>
#include <TM1638.h>

// WARNING: THIS EXAMPLE REQUIRES the TM1638 LIBRARY (https://github.com/rjbatista/tm1638-library/)
//
// This example is a basic clock; The time is displayed on the TM1638's 7 segments display.
// If you press the first button of the TM1638 for one seconde, the date will be displayed for 3 seconds.
// If you press the 8th button of the TM1638 for one second, the date will be adjusted to the closest minute
// (if seconds <30, they will be reseted to 0; if they are >=30 time will jump at the start of the next minute).
//
// If your DS1302 is not at the correct time, modify line 22 with the current time and uncomment line 17.
// After you started the sketch, comment again the line 17 and redownload the sketch or the clock will be
// reset every time your arduino reboot.

TM1638 module(8, 9, 7); // data=8, clock=9, strobe=7
DS1302 rtc(A2, A3, A1); // data(IO)=A2, clock(SCLK)=A3, strobe(CE)=A1
// #define SET_DATE_TIME_JUST_ONCE // Uncomment this to set the DS1302's clock.

void setup() {

#ifdef SET_DATE_TIME_JUST_ONCE  
  rtc.clockWrite(15,12,23,3,13,2,0); // Year(0-99), Month(1-12), Day (1-31), Day of week(1-7), Hour(0-24), Minute(0-60), Seconde(0-60)
#endif
}

void loop() {
  byte keys = module.getButtons();

  DS1302_clock* now = rtc.clockRead();

  char buffer[9];     
  if(keys&1) { // Display date
    module.setLEDs(1<<(now->wday-1));
    sprintf(buffer, "d=%02d%0d%02d", now->day, now->month, now->year);
    module.setDisplayToString(buffer, 0x14);

    delay(3000);
  }
  else { // Display time
    if(keys&0x80) { // Adjust clock
      rtc.clockWrite(now->year, now->month, now->day, now->wday, now->hours,
        now->minutes+((now->seconds>=30)?1:0), 0);
      now = rtc.clockRead();
    }
    module.setLEDs(0x80);
    sprintf(buffer, "t=%02d%02d%02d", now->hours, now->minutes, now->seconds);
    module.setDisplayToString(buffer, 0x14);
    delay(500);
    module.setDisplayToString(buffer, 0x00);
    delay(500);
  }
}

