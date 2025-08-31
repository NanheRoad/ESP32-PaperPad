/* Client side utilities for esp32-weather-epd.
 * Copyright (C) 2022-2024  Luke Marzen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

// built-in C++ libraries
#include <cstring>
#include <vector>

// arduino/esp32 libraries
#include <Arduino.h>
#include <esp_sntp.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <time.h>
#include <WiFi.h>

// additional libraries
#include <Adafruit_BusIO_Register.h>
#include <ArduinoJson.h>

// header files
#include "_locale.h"
#include "api_response.h"
#include "client_utils.h"
#include "config.h"
#include "display_utils.h"
#ifndef USE_HTTP
  #include <WiFiClientSecure.h>
#endif

#ifdef USE_HTTP
  static const uint16_t CMA_PORT = 80;
#else
  static const uint16_t CMA_PORT = 443;
#endif

/* 启动并连接WiFi
 * 接收一个int参数用于存储WiFi信号强度（RSSI）
 *
 * 返回WiFi连接状态
 */
wl_status_t startWiFi(int &wifiRSSI)
{
  WiFi.mode(WIFI_STA);
  Serial.printf("%s '%s'", TXT_CONNECTING_TO, WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // 如果WiFi在WIFI_TIMEOUT毫秒内未连接则超时
  unsigned long timeout = millis() + WIFI_TIMEOUT;
  wl_status_t connection_status = WiFi.status();

  while ((connection_status != WL_CONNECTED) && (millis() < timeout))
  {
    Serial.print(".");
    delay(50);
    connection_status = WiFi.status();
  }
  Serial.println();

  if (connection_status == WL_CONNECTED)
  {
    wifiRSSI = WiFi.RSSI(); // 现在获取WiFi信号强度，因为WiFi将被关闭以节省电量！
    Serial.println("IP地址: " + WiFi.localIP().toString());
  }
  else
  {
    Serial.printf("%s '%s'\n", TXT_COULD_NOT_CONNECT_TO, WIFI_SSID);
  }
  return connection_status;
} // startWiFi

/* 断开并关闭WiFi连接以节省电量
 */
void killWiFi()
{
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
} // killWiFi

/* 在串口监视器上打印本地时间
 *
 * 如果成功获取本地时间则返回true，否则返回false
 */
bool printLocalTime(tm *timeInfo)
{
  int attempts = 0;
  while (!getLocalTime(timeInfo) && attempts++ < 3)
  {
    Serial.println(TXT_FAILED_TO_GET_TIME);
    return false;
  }
  Serial.println(timeInfo, "%A, %B %d, %Y %H:%M:%S");
  return true;
} // printLocalTime

/* 等待NTP服务器时间同步，并根据config.cpp中指定的时区进行调整
 *
 * 如果时间设置成功则返回true，否则返回false
 *
 * 注意：必须连接WiFi才能从NTP服务器获取时间
 */
bool waitForSNTPSync(tm *timeInfo)
{
  // 等待SNTP同步完成
  unsigned long timeout = millis() + NTP_TIMEOUT;
  if ((sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET)
      && (millis() < timeout))
  {
    Serial.print(TXT_WAITING_FOR_SNTP);
    delay(100); // ms
    while ((sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET)
        && (millis() < timeout))
    {
      Serial.print(".");
      delay(100); // ms
    }
    Serial.println();
  }
  return printLocalTime(timeInfo);
} // waitForSNTPSync

/* 调用中国气象台天气预报API
 * 如果接收到数据，将解析并存入r参数中
 *
 * 返回HTTP状态码
 * 
 * 成功的状态码：
 *   200: HTTP_CODE_OK - 请求成功
 * 
 * 错误状态码：
 *   -512 ~ -520: WiFi连接错误（基于WiFi状态码偏移-512）
 *   -256 ~ -260: JSON解析错误（基于DeserializationError偏移-256）
 *   其他HTTP状态码: 标准HTTP错误码
 */
#ifdef USE_HTTP
  int getCMAweather(WiFiClient &client, cma_weather_t &r)
#else
  int getCMAweather(WiFiClientSecure &client, cma_weather_t &r)
#endif
{
  int attempts = 0;
  bool rxSuccess = false;
  DeserializationError jsonErr = {};
  // 构造请求URI
  String uri = String("/api/tianqi/tqyb.php?pid=") + CMA_PID + "&key=" + CMA_KEY
                + "&sheng=" + CMA_PROVINCE + "&shi=" + CMA_CITY
                + "&place=" + CMA_PLACE;

  String sanitizedUri = String(CMA_ENDPOINT) +
                        "/api/tianqi/tqyb.php?pid=" + CMA_PID + "&key={KEY}"
                        + "&sheng=" + CMA_PROVINCE + "&shi=" + CMA_CITY
                        + "&place=" + CMA_PLACE;

  Serial.print(TXT_ATTEMPTING_HTTP_REQ);
  Serial.println("：" + sanitizedUri);
  int httpResponse = 0;
  while (!rxSuccess && attempts < 3)
  {
    wl_status_t connection_status = WiFi.status();
    if (connection_status != WL_CONNECTED)
    {
      // -512 offset distinguishes these errors from httpClient errors
      return -512 - static_cast<int>(connection_status);
    }

    HTTPClient http;
    http.setConnectTimeout(HTTP_CLIENT_TCP_TIMEOUT); // default 5000ms
    http.setTimeout(HTTP_CLIENT_TCP_TIMEOUT); // default 5000ms
    http.begin(client, CMA_ENDPOINT, CMA_PORT, uri);
    httpResponse = http.GET();
    if (httpResponse == HTTP_CODE_OK)
    {
      jsonErr = deserializeCMAWeather(http.getStream(), r);
      if (jsonErr)
      {
        // -256 offset distinguishes these errors from httpClient errors
        httpResponse = -256 - static_cast<int>(jsonErr.code());
      }
      rxSuccess = !jsonErr;
    }
    client.stop();
    http.end();
    Serial.println("  " + String(httpResponse, DEC) + " "
                    + getHttpResponsePhrase(httpResponse));
    ++attempts;
  }

  return httpResponse;
} // getCMAweather