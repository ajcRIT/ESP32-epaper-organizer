#ifndef CONFIG_H
#define CONFIG_H

#include <string.h>
#include <GxEPD2_BW.h>


// ----------- Pins -----------
#define EPD_DC     6
#define EPD_CS     22
#define EPD_SCK    23
#define EPD_MOSI   5
#define EPD_RST     20
#define EPD_BUSY    19
#define ONBOARD_LED 15

// Wifi name and password
inline const char* ssid = "REDDYLLC-CHESTER";
inline const char* password = "Che$ter@29";

// URL to google calendar and tasks script (See README for instructions)
inline String calScriptUrl = "https://script.google.com/macros/s/AKfycbzj14kizI2lhFPq-zensrILQ7Yl5KtXEMLRQ7hx_Z9xBKb1lRXeMzNhy-u_zq7ajdcs/exec";
inline String taskScriptUrl = "https://script.google.com/macros/s/AKfycbzZznhWzbJWRDTH1w-YUr5-dbsYX-2nRXSFbxoc-nmNTirIczZRm85dHkb1alaoq6M_ng/exec";

// Time zone string (Find yours here: https://support.cyberdata.net/portal/en/kb/articles/010d63c0cfce3676151e1f2d5442e311)
inline const char* tz_string = "EST5EDT,M3.2.0,M11.1.0";

// Network time protocol server, for getting current date and time
inline const char* ntpServer = "pool.ntp.org";
inline const long  gmtOffset_sec = -18000;    // EST is GMT -5 hours (-5 * 3600 = -18000)
inline const int   daylightOffset_sec = 3600; // 1 hour daylight saving offset (in seconds)

// Status variables for tracking certain locations and data
inline bool calendarDataLoaded = false;
inline bool taskDataLoaded = false;
inline int taskYTracker = 85;
inline int todayHeaderPrinted = 0;
inline int tomorrowHeaderPrinted = 0;
inline int laterHeaderPrinted = 0;

inline GxEPD2_BW<GxEPD2_583_T8, GxEPD2_583_T8::HEIGHT> display(GxEPD2_583_T8(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

#endif