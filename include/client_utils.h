/* 客户端工具函数声明 */
#ifndef __CLIENT_UTILS_H__
#define __CLIENT_UTILS_H__

#include <Arduino.h>
#include "api_response.h"
#include "config.h"
#ifdef USE_HTTP
  #include <WiFiClient.h>
#else
  #include <WiFiClientSecure.h>
#endif

wl_status_t startWiFi(int &wifiRSSI);
void killWiFi();
bool waitForSNTPSync(tm *timeInfo);
bool printLocalTime(tm *timeInfo);
#ifdef USE_HTTP
  int getCMAweather(WiFiClient &client, cma_weather_t &r);
#else
  int getCMAweather(WiFiClientSecure &client, cma_weather_t &r);
#endif

#endif
