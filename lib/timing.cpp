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

long timePeriod (struct tm *startTime, struct tm *endTime){
  time_t startSeconds = mktime(startTime);
  time_t endSeconds = mktime(endTime);
  long elapsedMins = (endSeconds-startSeconds)/60;
  return elapsedMins;
}
