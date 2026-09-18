#include "timing.hpp"

/******************************************************
* @brief Convert UTC Time to a time_t value since epoch
* 
* @param tm The time structure to be converted
* @return time_t converted time_t value
********************************************************/
time_t convert_utc_tm_to_time_t(struct tm *tm) {
  char original_tz_buffer[64] = {0};
  char *original_tz = getenv("TZ");
  bool had_tz = false;
  if (original_tz) {
    had_tz = true;
    strncpy(original_tz_buffer, original_tz, sizeof(original_tz_buffer) - 1);
  }
  setenv("TZ", "UTC", 1);
  tzset();
  time_t utc_time = mktime(tm);
  if (had_tz) setenv("TZ", original_tz_buffer, 1);
  else unsetenv("TZ");
  tzset();
  return utc_time;
}

/***************************************************************
* @brief gets today's date from local network
* 
* @param date: array of characters to store date string in
*        dateSize: size of character array
* @return void
***************************************************************/
void getTodaysDate(char* date, size_t dateSize) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    strncpy(date, "No date", dateSize);
    date[dateSize - 1] = '\0';  // ensure null-termination
  } else {
    strftime(date, dateSize, "%m-%d-%Y", &timeinfo);
  }
}

/************************************************************
* @brief get tomorrow's date from local network
* 
* @param date: array of characters to store date string in
*        dateSize: size of character array
* @return void
**************************************************************/
void getTomorrowsDate(char* date, size_t dateSize) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    strncpy(date, "No date", dateSize);
    date[dateSize - 1] = '\0';  // ensure null-termination
  } else {
    time_t now = mktime(&timeinfo);      // convert struct tm -> time_t
    now += 24 * 60 * 60;                  // add one day (in seconds)
    struct tm* tomorrow = localtime(&now); // convert back to struct tm
    strftime(date, dateSize, "%m-%d-%Y", tomorrow);
  }
}

/************************************************************
* @brief returns the amount of time in minutes between two 
*        time structures
* 
* @param startTime: reference time
*        endTime: end time
* @return long: time in minutes between two entered times
**************************************************************/
long timePeriod (struct tm *startTime, struct tm *endTime){
  time_t startSeconds = mktime(startTime);
  time_t endSeconds = mktime(endTime);
  long elapsedMins = (endSeconds-startSeconds)/60;
  return elapsedMins;
}

// Returns days between date1 and date2 (positive if date1 is before date2)
/************************************************************
* @brief Compares the number of days between two dates
* 
* @param date1: reference date
*        date2: end date
* @return int: number of days between date1 and 2
**************************************************************/
int compareDates(struct tm date1, struct tm date2){
  struct tm date1Strip = {0}; 
  struct tm date2Strip = {0};
  date1Strip.tm_mon = date1.tm_mon;
  date1Strip.tm_mday =  date1.tm_mday;
  date1Strip.tm_year =  date1.tm_year;
  date2Strip.tm_mon = date2.tm_mon;
  date2Strip.tm_mday =  date2.tm_mday;
  date2Strip.tm_year =  date2.tm_year;
  time_t date1Seconds = mktime(&date1Strip);
  time_t date2Seconds = mktime(&date2Strip);
  int elapsedDays = (date2Seconds-date1Seconds)/86400;
  return elapsedDays;
} 
