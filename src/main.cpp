#include <WiFi.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include "timing.hpp"
#include "display_helpers.hpp"
#include "config.hpp"
#include "http_operations.hpp"
#include "display_main.hpp"

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_TIMER: 
      Serial.println("Wakeup caused by timer (24-hour interval reached)."); 
      break;
    case ESP_SLEEP_WAKEUP_EXT1:
      Serial.println("Wakeup caused by external button press (ext1)."); 
      break;
    default: 
      Serial.printf("Wakeup reason code: %d\n", wakeup_reason); 
      break;
  }
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
        drawTask(item);
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
        drawEvent(item);
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
  esp_sleep_enable_ext1_wakeup(pin_bitmask, ESP_EXT1_WAKEUP_ANY_LOW);
  gpio_set_direction((gpio_num_t)REFRESH, GPIO_MODE_INPUT);
  gpio_pullup_en((gpio_num_t)REFRESH);
  gpio_pulldown_dis((gpio_num_t)REFRESH);

  // Enter Deep Sleep
  esp_deep_sleep_start();
}

void loop(){

}