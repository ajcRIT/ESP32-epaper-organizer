#ifndef DISPLAY_HELPERS_HPP
#define DISPLAY_HELPERS_HPP

#include <string.h>
#include <Adafruit_GFX.h>
#include <GxEPD2_BW.h>
#include "config.hpp"

String removeAccents(String);
int getBatteryPercent();
int printSmartWrap(String, int, int, int, int);
void drawDottedLineHorizontal (int, int, int, int);
void drawCheckMark (int, int, int);
void drawChargeSymbol(int, int);


#endif