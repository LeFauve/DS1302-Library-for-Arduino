#include <DS1302.h>

// This example is a basic clock; The time and date are displayed on the serial console every second
//
// If your DS1302 is not at the correct time, modify line 14 with the current time and uncomment line 10.
// After you started the sketch, comment again the line 10 and redownload the sketch or the clock will be
// reset every time your arduino reboot.

DS1302 rtc(A2, A3, A1); // data(IO)=A2, clock(SCLK)=A3, strobe(CE)=A1
// #define SET_DATE_TIME_JUST_ONCE // uncomment this to set the DS1302's clock

void setup() {
#ifdef SET_DATE_TIME_JUST_ONCE  
  rtc.clockWrite(15,12,23,3,13,2,0); // Year(0-99), Month(1-12), Day (1-31), Day of week(1-7), Hour(0-24), Minute(0-60), Seconde(0-60)
#endif

  Serial.begin(9600); // initialize serial communication at 9600 bits per second:
}

void loop() {
  DS1302_clock* now = rtc.clockRead();

  char buffer[32];     
  sprintf(buffer, "Date: %02d/%0d/%02d\nTime: %02d:%02d:%02d\n\n",
    now->day, now->month, now->year, now->hours, now->minutes, now->seconds);
  Serial.println(buffer);
  delay(1000);
}

