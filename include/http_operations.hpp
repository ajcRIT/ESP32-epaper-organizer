#ifndef HTTP_OPERATIONS_H
#define HTTP_OPERATIONS_H

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.hpp"

JsonDocument fetchCalendarData();
JsonDocument fetchTaskData();

#endif