#include "http_operations.hpp"


JsonDocument fetchCalendarData() {
  JsonDocument calendarJson;
  JsonDocument doc;
  Serial.println("Fetching calendar data...");
  HTTPClient http;
  http.begin(calScriptUrl);
  http.setTimeout(20000);
  http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  int httpCode = http.GET();
  calendarDataLoaded = false;
  if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      DeserializationError error = deserializeJson(calendarJson, payload);
      if (error) {
        Serial.println(error.c_str());
        deserializeJson(doc, "");
      }
      else { 
        calendarDataLoaded = true;
        return calendarJson;
      }
  } else {
    Serial.println("No payload recieved");
    deserializeJson(doc, "");
  }
  http.end();
  return doc;
}

JsonDocument fetchTaskData() {
  JsonDocument taskJson;
  JsonDocument doc;
  Serial.println("Fetching task data...");
  HTTPClient http;
  http.begin(taskScriptUrl);
  http.setTimeout(20000);
  http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  int httpCode = http.GET();
  taskDataLoaded = false;
  if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      DeserializationError error = deserializeJson(taskJson, payload);
      if (error) {
        Serial.println(error.c_str());
        deserializeJson(doc, "");
      }
      else { 
        taskDataLoaded = true;
        return taskJson;
      }
  } else {
    Serial.println("No payload recieved");
    deserializeJson(doc, "");
  }
  http.end();
  return doc;
}