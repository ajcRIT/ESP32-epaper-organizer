#ifndef TIMING_HPP
#define TIMING_HPP

#include "time.h"
#include <string.h>
#include <Wifi.h>

time_t convert_utc_tm_to_time_t(struct tm*);
void getTodaysDate(char*, size_t);
void getTomorrowsDate(char*, size_t);
long timePeriod (struct tm*, struct tm*);
int compareDates(struct tm, struct tm);

#endif