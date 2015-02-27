#include <x86/rtc.h>
#include <x86/asm.h>

int getrtcfield (int field) {
  outb(RTC_PORT_OUT, field);
  int bcd = inb(RTC_PORT_IN);
  return (bcd & 0xF) + (bcd >> 4)*10;
}

void gettime (time_t *time) {
  time->year = getrtcfield(RTC_YEAR);
  time->month = getrtcfield(RTC_MONTH);
  time->day = getrtcfield(RTC_DAY);
  time->hour = getrtcfield(RTC_HOURS);
  time->minute = getrtcfield(RTC_MINS);
  time->second = getrtcfield(RTC_SECS);
}
