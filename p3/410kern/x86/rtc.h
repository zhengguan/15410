#ifndef _RTC_H_
#define _RTC_H_

#define RTC_SECS  0
#define RTC_MINS  2
#define RTC_HOURS 4
#define RTC_DAY   7
#define RTC_MONTH 8
#define RTC_YEAR  9

#define RTC_PORT_OUT 0x70
#define RTC_PORT_IN  0x71

typedef struct time {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
} time_t;

void gettime(time_t *time);

void printtime();

#endif /* _RTC_H_ */
