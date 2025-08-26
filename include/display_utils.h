/* 显示辅助函数声明 */
#ifndef __DISPLAY_UTILS_H__
#define __DISPLAY_UTILS_H__

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

uint32_t readBatteryVoltage();
uint32_t calcBatPercent(uint32_t v, uint32_t minv, uint32_t maxv);
const uint8_t *getBatBitmap24(uint32_t batPercent);
void getDateStr(String &s, tm *timeInfo);
void getRefreshTimeStr(String &s, bool timeSuccess, tm *timeInfo);
const char *getWiFidesc(int rssi);
const uint8_t *getWiFiBitmap16(int rssi);
const char *getHttpResponsePhrase(int code);
const char *getWifiStatusPhrase(wl_status_t status);
void printHeapUsage();
void disableBuiltinLED();

#endif
