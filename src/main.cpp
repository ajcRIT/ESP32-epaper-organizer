#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <SPI.h>
#include "time.h"

const char* ssid = "REDDYLLC-CHESTER";
const char* password = "Che$ter@29";
String calScriptUrl = "https://script.google.com/macros/s/AKfycbzj14kizI2lhFPq-zensrILQ7Yl5KtXEMLRQ7hx_Z9xBKb1lRXeMzNhy-u_zq7ajdcs/exec";
String taskScriptUrl = "https://script.google.com/macros/s/AKfycbzZznhWzbJWRDTH1w-YUr5-dbsYX-2nRXSFbxoc-nmNTirIczZRm85dHkb1alaoq6M_ng/exec";
const char* tz_string = "EST5EDT,M3.2.0,M11.1.0";
bool calendarDataLoaded = false;
bool taskDataLoaded = false;
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000;    // EST is GMT -5 hours (-5 * 3600 = -18000)
const int   daylightOffset_sec = 3600; // 1 hour daylight saving offset (in seconds)
int taskYTracker = 85;
int todayHeaderPrinted = 0;
int tomorrowHeaderPrinted = 0;
int laterHeaderPrinted = 0;

// ----------- Pins -----------
#define EPD_DC     6
#define EPD_CS     22
#define EPD_SCK    23
#define EPD_MOSI   5
#define EPD_RST     20
#define EPD_BUSY    19
#define ONBOARD_LED 15

GxEPD2_BW<GxEPD2_583_T8, GxEPD2_583_T8::HEIGHT> display(GxEPD2_583_T8(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));



// Robust UTC Time Conversion
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

String removeAccents(String str) {
  str.replace("é", "e");
  str.replace("è", "e");
  str.replace("ê", "e");
  str.replace("à", "a");
  str.replace("ç", "c");
  return str;
}

int getBatteryPercent() {
  int pin = 0;
  analogReadResolution(12);
  analogSetPinAttenuation((gpio_num_t)pin, ADC_11db);
  int mv = analogReadMilliVolts(pin);
  int batteryV = mv * 2;

  int percent = map(batteryV, 3100, 4100, 0, 100);
  return constrain(percent, 0, 100);
}

int printSmartWrap(String text, int x, int y, int maxChars, int _lineOffset) {
  int currentPos = 0;
  int lineOffset = 0;
  int numLines = 0;
  
  while (currentPos < text.length()) {
    String line = text.substring(currentPos, currentPos + maxChars);

    if (currentPos + maxChars < text.length()) {
      int lastSpace = line.lastIndexOf(' ');
      if (lastSpace != -1) {
        line = line.substring(0, lastSpace);
        currentPos += (lastSpace + 1);
      } else {
        currentPos += maxChars;
      }
    } else {
      currentPos += maxChars;
    }
    
    display.setCursor(x, y + lineOffset);
    display.print(line);
    lineOffset += _lineOffset;
    numLines++;
  }
  return numLines;
}

DynamicJsonDocument fetchCalendarData() {
  DynamicJsonDocument calendarJson (12000);
  Serial.println("Fetching calendar data...");
  HTTPClient http;
  http.begin(calScriptUrl);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET();
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
      String payload = http.getString();
      DeserializationError error = deserializeJson(calendarJson, payload);
      if (error) {
        calendarDataLoaded = false;
        DynamicJsonDocument doc(1);
        deserializeJson(doc, payload);
        return doc;
      }
      else { 
        calendarDataLoaded = true;
        return calendarJson;
      }
    }
  } else {
  calendarDataLoaded = false;
  }
  http.end();
  DynamicJsonDocument doc(1);
  deserializeJson(doc, "");
  return doc;
}

DynamicJsonDocument fetchTaskData() {
  DynamicJsonDocument taskJson (12000);
  Serial.println("Fetching task data...");
  HTTPClient http;
  http.begin(taskScriptUrl);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET();
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
      String payload = http.getString();
      DeserializationError error = deserializeJson(taskJson, payload);
      if (error) {
        taskDataLoaded = false;
        DynamicJsonDocument doc(1);
        deserializeJson(doc, payload);
        return doc;
      }
      else { 
        taskDataLoaded = true;
        return taskJson;
      }
    }
  } else {
  taskDataLoaded = false;
  }
  http.end();
  DynamicJsonDocument doc(1);
  deserializeJson(doc, "");
  return doc;
}


void drawDottedLineHorizontal (int x, int y, int length, int dashLength){
  int cur_x = x;
  do{
    display.drawLine(cur_x, y, cur_x+dashLength, y, GxEPD_BLACK);
    cur_x += (dashLength*2);
  } while (cur_x < length);
}

void drawCheckMark (int x, int y, int size){
  int pivotX = x + (size * 0.3);
  int pivotY = y + (size * 0.8);
  int endX = x + size;
  int endY = y + (size * 0.2);

  // Draw the short left-side downward stroke
  display.drawLine(x, y + (size * 0.5), pivotX, pivotY, GxEPD_BLACK);
  
  // Draw the longer right-side upward stroke
  display.drawLine(pivotX, pivotY, endX, endY, GxEPD_BLACK);
}

void drawChargeSymbol(int x, int y){
  int x0 = x + 22, y0 = y + 3;  // Top point
  int x1 = x + 14, y1 = y + 11; // Left-most point
  int x2 = x + 20, y2 = y + 11; // Center line right point
  display.fillTriangle(x0, y0, x1, y1, x2, y2, GxEPD_WHITE);

  // Bottom Half of the Bolt
  int x3 = x + 18, y3 = y + 9;  // Center line left point
  int x4 = x + 24, y4 = y + 9;  // Right-most point
  int x5 = x + 16, y5 = y + 17; // Bottom tip point
  display.fillTriangle(x3, y3, x4, y4, x5, y5, GxEPD_WHITE);
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
// Returns days between date1 and date2 (positive if date1 is before date2)
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


long timePeriod (struct tm *startTime, struct tm *endTime){
  time_t startSeconds = mktime(startTime);
  time_t endSeconds = mktime(endTime);
  long elapsedMins = (endSeconds-startSeconds)/60;
  return elapsedMins;
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
  strftime(date, sizeof(date), "%m-%d-%Y", &startTime);
  char getdate [30];
  char tomorrow [30];
  getTodaysDate(getdate, sizeof(getdate));
  getTomorrowsDate(tomorrow, sizeof(tomorrow));

  int blockLocationY = 75;
  int blockLocationX = 20;
  int blockSize = 0;
  int blockWidth = 0;
  if(String(getdate) == String(date)){
    blockLocationY += (startTimeHour-7)*(25);
    float blockCalc = (float(length)/60)*25; 
    blockSize = ceil(blockCalc);
    blockWidth = 170;
    Serial.println(title);
    Serial.println(String(date));
    Serial.println(String(length));
    Serial.println(blockCalc);
    Serial.println(blockSize);
    Serial.println("");
  }else if (String(tomorrow) == String(date)){
    blockLocationY += (startTimeHour-7)*(25);
    blockLocationX += 190;
    blockSize = ceil(float(length)/60)*25;
    blockWidth = 180;
    Serial.println(title);
    Serial.println(String(date));
    Serial.println(String(length));
    Serial.println("");
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
  configTzTime(tz_string, ntpServer);
}

void loop() {
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
  DynamicJsonDocument cal(12000);
  DynamicJsonDocument task(12000);
  cal = fetchCalendarData();
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
  }
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
    Serial.println("HTTP Error");
  }
  
  display.powerOff();
  delay(300000);
}