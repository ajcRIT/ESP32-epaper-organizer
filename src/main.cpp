#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <SPI.h>
#include <Arduino.h>
#include "time.h"
#include "timing.hpp"
#include "display_helpers.hpp"
#include "config.hpp"

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_TIMER: 
      Serial.println("Wakeup caused by timer (24-hour interval reached)."); 
      break;
    case ESP_SLEEP_WAKEUP_EXT1: // Changed from EXT0 to EXT1
      Serial.println("Wakeup caused by external button press (ext1)."); 
      break;
    default: 
      Serial.printf("Wakeup reason code: %d\n", wakeup_reason); 
      break;
  }
}

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

void drawOutline(){
  int batCap = getBatteryPercent();
  int segments = map(batCap, 0, 100, 0, 5);

  display.setPartialWindow(0, 0, display.width(), display.height());
  display.firstPage();

  display.fillScreen(GxEPD_WHITE);
  display.fillRect(0, 0, 648, 35, GxEPD_BLACK);
  display.setTextColor(GxEPD_WHITE);
  display.setFont(NULL);
  display.setTextSize(2);
  display.setCursor(10, 10);
  char date [30];
  getTodaysDate(date, sizeof(date));
  display.print(String(date));
  int batX = 600;
  int batY = 8;
  display.drawRect(batX, batY, 40, 16, GxEPD_WHITE);
  display.fillRect(batX+40, batY+4, 3, 7, GxEPD_WHITE);
  display.setCursor(batX-30, batY+4);
  display.setTextSize(1);
  display.print(String(batCap) + "%");
  if(batCap <= 10){
    display.setCursor(batX-40, batY+2);
    display.setTextSize(2);
    display.print("!");
  }
  
  for(int i=0; i<segments; i++) {
    display.fillRect(batX+4+(i*7), batY+4, 4, 8, GxEPD_WHITE);
  }
  
  display.drawRect(0, 35, 200, 40, GxEPD_BLACK);
  display.setTextColor(GxEPD_BLACK);
  display.setCursor(10, 50);
  display.setTextSize(2);
  display.print("Today");
  display.drawRect(200, 35, 200, 40, GxEPD_BLACK);
  display.setTextColor(GxEPD_BLACK);
  display.setCursor(210, 50);
  display.print("Tomorrow");
  display.drawRect(400, 35, 248, 40, GxEPD_BLACK);
  display.setTextColor(GxEPD_BLACK);
  display.setCursor(410, 50);
  display.print("Tasks");
  display.drawLine(200, 75, 200, 480, GxEPD_BLACK);
  display.drawLine(400, 75, 400, 480, GxEPD_BLACK);
  for(int i = 1; i <= 17; i++){
    drawDottedLineHorizontal(15, 75+(i*25), 400, 5);
    display.setCursor(0, 70+(i*25));
    display.setTextSize(1);
    if(i<=5){
      String time = String(i+7);
      display.print(time);
    } else {
      String time = String(i-5);
      display.print(time);
    }
  }
  do {
  } while (display.nextPage());
}


void printTask(String title, String due, bool done){
  int taskLocationX = 430;
  int taskLocationY = taskYTracker;
  if (taskLocationY < 470){
    display.setPartialWindow(taskLocationX-20, taskLocationY, 228, 100);
    display.firstPage();
    display.setTextSize(1);
    display.drawRect(taskLocationX-20, taskLocationY, 10, 10, GxEPD_BLACK);
    int yShift = printSmartWrap(title, taskLocationX, taskLocationY, 35, 10);
    yShift += printSmartWrap("Due:" + due, taskLocationX, taskLocationY+(10*yShift), 35, 10);
    if (done){
      drawCheckMark(taskLocationX-20, taskLocationY-5, 15);
    }
    do {

    } while (display.nextPage());
    taskYTracker += 10+(yShift*10);
  } else {
    return;
  }
}

void drawTask(JsonObject tasks, int index, int total){
  String title = tasks["title"].as<String>();
  bool done = (tasks["status"].as<String>() == "completed" ? true : false);
  struct tm dueDate = {};
  strptime(tasks["due"].as<const char*>(), "%FT%TZ", &dueDate);
  dueDate.tm_isdst = -1;
  char due [30];
  strftime(due, sizeof(due), "%m-%d-%Y", &dueDate);
  struct tm today;
  getLocalTime(&today);
  int daysFromDue = compareDates(today, dueDate);
  if (daysFromDue < 0 && done){
    return;
  } else if (daysFromDue < 0 && !done){
    printTask(title, String(due), done);
    return;
  } else if (todayHeaderPrinted == 0 && daysFromDue == 0){
    todayHeaderPrinted = 1;
    display.setPartialWindow(410, taskYTracker-3, 228, 15);
    display.firstPage();
    display.setCursor(410, taskYTracker-3);
    display.print("Today");
    display.drawLine(450, taskYTracker, 640, taskYTracker, GxEPD_BLACK);
    taskYTracker += 10;
    do {

    } while (display.nextPage());
    printTask(title, String(due), done);
    return;
  } else if (tomorrowHeaderPrinted == 0 && daysFromDue == 1){
    tomorrowHeaderPrinted = 1;
    display.setPartialWindow(410, taskYTracker-3, 228, 15);
    display.firstPage();
    display.setCursor(410, taskYTracker-3);
    display.print("Tomorrow");
    display.drawLine(460, taskYTracker, 640, taskYTracker, GxEPD_BLACK);
    taskYTracker += 10;
    do {

    } while (display.nextPage());
    printTask(title, String(due), done);
    return;
  } else if (laterHeaderPrinted == 0 && daysFromDue > 1){
    laterHeaderPrinted = 1;
    display.setPartialWindow(410, taskYTracker-3, 228, 15);
    display.firstPage();
    display.setCursor(410, taskYTracker-3);
    display.print("Later");
    display.drawLine(450, taskYTracker, 640, taskYTracker, GxEPD_BLACK);
    taskYTracker += 10;
    do {

    } while (display.nextPage());
    printTask(title, String(due), done);
    return;
  } else {
    printTask(title, String(due), done);
    return;
  }
}

void drawTodo(JsonObject obj, int index, int total) {
  String title = obj["title"].as<String>();
  struct tm startTime = {};
  struct tm endTime = {};
  strptime(obj["startTime"].as<const char*>(), "%FT%TZ", &startTime);
  strptime(obj["endTime"].as<const char*>(), "%FT%TZ", &endTime);
  startTime.tm_isdst = -1;
  endTime.tm_isdst = -1;

  long length = timePeriod(&startTime, &endTime);
  char date [30];
  time_t epoch = convert_utc_tm_to_time_t(&startTime);        // UTC struct -> UTC epoch (no offset applied)
  struct tm *local = localtime(&epoch);  // epoch -> local struct tm (DST-aware, uses configTime settings)
  float startTimeHour = local->tm_hour + local->tm_min / 60.0f;
  if (local->tm_hour >= 20){ // Adjust the date if UTC offset put start time one day ahead
    startTime.tm_mday -= 1;
  }
  strftime(date, sizeof(date), "%m-%d-%Y", &startTime);
  char getdate [30];
  char tomorrow [30];
  getTodaysDate(getdate, sizeof(getdate));
  getTomorrowsDate(tomorrow, sizeof(tomorrow));
  Serial.println(title);
  Serial.println(local->tm_hour);
  Serial.println(startTimeHour);
  Serial.println(String(date));
  Serial.println("");

  int blockLocationY = 75;
  int blockLocationX = 20;
  int blockSize = 0;
  int blockWidth = 0;
  if(String(getdate) == String(date)){
    blockLocationY += (startTimeHour-7)*(25);
    float blockCalc = (float(length)/60)*25; 
    blockSize = ceil(blockCalc);
    blockWidth = 170;

  }else if (String(tomorrow) == String(date)){
    blockLocationY += (startTimeHour-7)*(25);
    blockLocationX += 190;
    blockSize = ceil(float(length)/60)*25;
    blockWidth = 180;
  } else {
    return;
  }
  
  display.setPartialWindow(blockLocationX, blockLocationY, blockWidth, blockSize);
  display.firstPage();
  display.drawRect(blockLocationX, blockLocationY, blockWidth, blockSize, GxEPD_BLACK);
  display.setCursor(blockLocationX+20, blockLocationY+20);
  printSmartWrap(title, blockLocationX+5, blockLocationY+5, 30, 10);
  do {

  } while (display.nextPage());
}


void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {
    delay(10);
  }

  print_wakeup_reason();
  pinMode(ONBOARD_LED, OUTPUT);
  digitalWrite(ONBOARD_LED, LOW);
  delay(100);

  SPI.begin(EPD_SCK, -1, EPD_MOSI, EPD_CS);
  display.init(115200);
  display.setRotation(0); 
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  } while (display.nextPage());
  
  WiFi.begin(ssid, password);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  configTzTime(tz_string, ntpServer);\

  taskYTracker = 85;
  todayHeaderPrinted = 0;
  tomorrowHeaderPrinted = 0;
  laterHeaderPrinted = 0;
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  } while (display.nextPage());
  if (WiFi.status() != WL_CONNECTED) {
    delay(500);
    digitalWrite(ONBOARD_LED, HIGH);
    delay(500);
    digitalWrite(ONBOARD_LED, LOW);
    return;
  }
  digitalWrite(ONBOARD_LED, HIGH);
  drawOutline();
  JsonDocument cal;
  JsonDocument task;
  task = fetchTaskData();
  if (taskDataLoaded){
    JsonArray array = task.as<JsonArray>();
    if (array.size() > 0) {
      int current = 1;
      for (JsonObject item : array) {
        drawTask(item, current, array.size());
        current++;
      }
    }
  } else {
    Serial.println("HTTP Error fetching tasks");
    return;
  }
  cal = fetchCalendarData();
  if (calendarDataLoaded) {
    JsonArray array = cal.as<JsonArray>();
    if (array.size() > 0) {
      int current = 1;
      for (JsonObject item : array) {
        drawTodo(item, current, array.size());
        current++;
      }
    }
  } else {
    Serial.println("HTTP Error fetching calendar events");
    return;
  }
  
  display.powerOff();
  Serial.println("Going to sleep now...");
  Serial.flush();

  // 1. Configure Timer Wakeup (24 Hours)
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP);
  // Create a 64-bit bitmask representing the REFRESH pin
  uint64_t pin_bitmask = (1ULL << REFRESH);
  // ESP_EXT1_WAKEUP_ALL_LOW wakes up when the pin hits GND
  esp_sleep_enable_ext1_wakeup(pin_bitmask, ESP_EXT1_WAKEUP_ALL_LOW);
  gpio_set_direction((gpio_num_t)REFRESH, GPIO_MODE_INPUT);
  gpio_pullup_en((gpio_num_t)REFRESH);
  gpio_pulldown_dis((gpio_num_t)REFRESH);

  // Enter Deep Sleep
  esp_deep_sleep_start();
}