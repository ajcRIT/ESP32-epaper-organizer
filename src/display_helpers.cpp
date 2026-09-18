#include "display_helpers.hpp"

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

