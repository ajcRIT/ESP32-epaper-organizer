#ifndef DISPLAY_MAIN_H
#define DISPLAY_MAIN_H

#include "config.hpp"
#include "display_helpers.hpp"
#include "timing.hpp"
#include <string.h>
#include <ArduinoJson.h>


void drawOutline();
void drawTask(JsonObject);
void drawEvent(JsonObject);
void printTask();


#endif