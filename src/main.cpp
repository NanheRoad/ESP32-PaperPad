/* esp32-weather-epd 主程序
 * Copyright (C) 2022-2025  Luke Marzen
 *
 * 本程序为自由软件：你可以根据自由软件基金会发布的 GNU 通用公共许可证第 3 版，
 * 或（由你选择）任何更高版本，重新发布和/或修改本程序。
 *
 * 本程序的发布是希望它能发挥作用，但没有任何担保；
 * 甚至没有适销性或特定用途适用性的隐含担保。更多细节请参阅 GNU 通用公共许可证。
 *
 * 你应该已经收到随本程序一起提供的 GNU 通用公共许可证副本。
 * 如果没有，请参阅 <https://www.gnu.org/licenses/>。
 */

#include <Arduino.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <pcf8563.h>
#include <Preferences.h>
#include <time.h>
#include <WiFi.h>

#include "_locale.h"
#include "api_response.h"
#include "client_utils.h"
#include "config.h"
#include "display_utils.h"
#include "icons/icons_196x196.h"
#include "renderer.h"
#if defined(USE_HTTPS_WITH_CERT_VERIF) || defined(USE_HTTPS_WITH_CERT_VERIF)
  #include <WiFiClientSecure.h>
#endif
#ifdef USE_HTTPS_WITH_CERT_VERIF
  #include "cert.h"
#endif

// 太大，无法在栈上分配
static owm_resp_onecall_t       weather_data;
static owm_resp_air_pollution_t owm_air_pollution;
// 使用 lewisxhe/PCF8563_Library 驱动 BL8025C 实时时钟
static PCF8563_Class rtc; // 外部 RTC

Preferences prefs;


/* 让 esp32 进入超低功耗深度睡眠（<11μA）。
 * 唤醒时间对齐到分钟。睡眠时间在 config.cpp 中定义。
 */
void beginDeepSleep(unsigned long startTime, tm *timeInfo)
{
  if (!getLocalTime(timeInfo))
  {
    Serial.println(TXT_REFERENCING_OLDER_TIME_NOTICE);
  }

  // 为简化睡眠时间计算，当前由 timeInfo 存储的时间将被转换为相对于 WAKE_TIME 的时间。
  // 这样，如果 SLEEP_DURATION 不是 60 分钟的倍数，可以更容易地对齐，
  // 并且可以轻松判断是否需要因睡觉时间而额外睡眠。
  // 例如，当 curHour == 0 时，timeInfo->tm_hour == WAKE_TIME
  int bedtimeHour = INT_MAX;
  if (BED_TIME != WAKE_TIME)
  {
    bedtimeHour = (BED_TIME - WAKE_TIME + 24) % 24;
  }

  // 时间相对于唤醒时间
  int curHour = (timeInfo->tm_hour - WAKE_TIME + 24) % 24;
  const int curMinute = curHour * 60 + timeInfo->tm_min;
  const int curSecond = curHour * 3600
                      + timeInfo->tm_min * 60
                      + timeInfo->tm_sec;
  const int desiredSleepSeconds = SLEEP_DURATION * 60;
  const int offsetMinutes = curMinute % SLEEP_DURATION;
  const int offsetSeconds = curSecond % desiredSleepSeconds;

  // 唤醒时间对齐到 SLEEP_DURATION 的最近倍数
  int sleepMinutes = SLEEP_DURATION - offsetMinutes;
  if (desiredSleepSeconds - offsetSeconds < 120
   || offsetSeconds / (float)desiredSleepSeconds > 0.95f)
  { // 如果睡眠时间少于 2 分钟或少于 SLEEP_DURATION 的 5%，则跳到下一个对齐点
    sleepMinutes += SLEEP_DURATION;
  }

  // 预计唤醒时间，如果落在睡眠区间则需要调整 sleepDuration
  const int predictedWakeHour = ((curMinute + sleepMinutes) / 60) % 24;

  uint64_t sleepDuration;
  if (predictedWakeHour < bedtimeHour)
  {
    sleepDuration = sleepMinutes * 60 - timeInfo->tm_sec;
  }
  else
  {
    const int hoursUntilWake = 24 - curHour;
    sleepDuration = hoursUntilWake * 3600ULL
                    - (timeInfo->tm_min * 60ULL + timeInfo->tm_sec);
  }

  // 增加额外延迟以补偿部分 esp32 RTC 过快的问题
  sleepDuration += 3ULL;
  sleepDuration *= 1.0015f;

#if DEBUG_LEVEL >= 1
  printHeapUsage();
#endif

  esp_sleep_enable_timer_wakeup(sleepDuration * 1000000ULL);
  Serial.print(TXT_AWAKE_FOR);
  Serial.println(" "  + String((millis() - startTime) / 1000.0, 3) + "s");
  Serial.print(TXT_ENTERING_DEEP_SLEEP_FOR);
  Serial.println(" " + String(sleepDuration) + "s");
  esp_deep_sleep_start();
} // end beginDeepSleep

/* 程序入口
 */
void setup()
{
  unsigned long startTime = millis();
  Serial.begin(115200);

  // 初始化 I2C 与外部实时时钟
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  rtc.begin();

#if DEBUG_LEVEL >= 1
  printHeapUsage();
#endif

  disableBuiltinLED();

  // 打开命名空间以读写非易失性存储
  prefs.begin(NVS_NAMESPACE, false);

#if BATTERY_MONITORING
  uint32_t batteryVoltage = readBatteryVoltage();
  Serial.print(TXT_BATTERY_VOLTAGE);
  Serial.println(": " + String(batteryVoltage) + "mv");


  // 当电池电量低时，应该刷新显示，但只在首次检测到低电压时刷新。
  // 下次刷新是在电压恢复正常时。为此我们会用非易失性存储做标记。
  bool lowBat = prefs.getBool("lowBat", false);

  // 电池电量低，立即进入深度睡眠
  if (batteryVoltage <= LOW_BATTERY_VOLTAGE)
  {
    if (lowBat == false)
    {  // 首次检测到电池电量低
      prefs.putBool("lowBat", true);
      prefs.end();
      initDisplay();
      do
      {
        drawError(battery_alert_0deg_196x196, TXT_LOW_BATTERY);
      } while (display.nextPage());
      powerOffDisplay();
    }

    if (batteryVoltage <= CRIT_LOW_BATTERY_VOLTAGE)
    { // 严重低电压
      // 不设置 esp_sleep_enable_timer_wakeup();
      // 只有手动按下 RST 按钮才会唤醒
      Serial.println(TXT_CRIT_LOW_BATTERY_VOLTAGE);
      Serial.println(TXT_HIBERNATING_INDEFINITELY_NOTICE);
    }
    else if (batteryVoltage <= VERY_LOW_BATTERY_VOLTAGE)
    { // 非常低电压
      esp_sleep_enable_timer_wakeup(VERY_LOW_BATTERY_SLEEP_INTERVAL
                                    * 60ULL * 1000000ULL);
      Serial.println(TXT_VERY_LOW_BATTERY_VOLTAGE);
      Serial.print(TXT_ENTERING_DEEP_SLEEP_FOR);
      Serial.println(" " + String(VERY_LOW_BATTERY_SLEEP_INTERVAL) + "min");
    }
    else
    {  // 低电压
      esp_sleep_enable_timer_wakeup(LOW_BATTERY_SLEEP_INTERVAL
                                    * 60ULL * 1000000ULL);
      Serial.println(TXT_LOW_BATTERY_VOLTAGE);
      Serial.print(TXT_ENTERING_DEEP_SLEEP_FOR);
      Serial.println(" " + String(LOW_BATTERY_SLEEP_INTERVAL) + "min");
    }
    esp_deep_sleep_start();
  }
  // 电池恢复正常，重置非易失性存储变量
  if (lowBat == true)
  {
    prefs.putBool("lowBat", false);
  }
#else
  uint32_t batteryVoltage = UINT32_MAX;
#endif

  // 所有数据已从 NVS 加载，关闭文件系统
  prefs.end();

  String statusStr = {};
  String tmpStr = {};
  tm timeInfo = {};

  // START WIFI
  int wifiRSSI = 0; // “接收信号强度指示器"
  wl_status_t wifiStatus = startWiFi(wifiRSSI);
  if (wifiStatus != WL_CONNECTED)
  { // WiFi 连接失败
    killWiFi();
    initDisplay();
    if (wifiStatus == WL_NO_SSID_AVAIL)
    {
      Serial.println(TXT_NETWORK_NOT_AVAILABLE);
      do
      {
        drawError(wifi_x_196x196, TXT_NETWORK_NOT_AVAILABLE);
      } while (display.nextPage());
    }
    else
    {
      Serial.println(TXT_WIFI_CONNECTION_FAILED);
      do
      {
        drawError(wifi_x_196x196, TXT_WIFI_CONNECTION_FAILED);
      } while (display.nextPage());
    }
    powerOffDisplay();
    beginDeepSleep(startTime, &timeInfo);
  }

  // 时间同步
  configTzTime(TIMEZONE, NTP_SERVER_1, NTP_SERVER_2);
  bool timeConfigured = waitForSNTPSync(&timeInfo);
  if (!timeConfigured)
  {
    Serial.println(TXT_TIME_SYNCHRONIZATION_FAILED);
    // 使用外部 RTC 时间作为回退
    auto now = rtc.getDateTime();
    timeInfo.tm_year = now.year - 1900;
    timeInfo.tm_mon  = now.month - 1;
    timeInfo.tm_mday = now.day;
    timeInfo.tm_hour = now.hour;
    timeInfo.tm_min  = now.minute;
    timeInfo.tm_sec  = now.second;
  }

  // API 请求
#ifdef USE_HTTP
  WiFiClient client;
#elif defined(USE_HTTPS_NO_CERT_VERIF)
  WiFiClientSecure client;
  client.setInsecure();
#elif defined(USE_HTTPS_WITH_CERT_VERIF)
  WiFiClientSecure client;
  client.setCACert(cert_Sectigo_RSA_Domain_Validation_Secure_Server_CA);
#endif
  int rxStatus = getCMAweather(client, weather_data);
  if (rxStatus != HTTP_CODE_OK)
    {
      killWiFi();
      statusStr = "CMA Weather API";
      tmpStr = String(rxStatus, DEC) + ": " + getHttpResponsePhrase(rxStatus);
    initDisplay();
    do
    {
      drawError(wi_cloud_down_196x196, statusStr, tmpStr);
    } while (display.nextPage());
    powerOffDisplay();
    beginDeepSleep(startTime, &timeInfo);
  }
    killWiFi();  // WiFi 不再需要

  // 读取室内温湿度，使用 SHT30 传感器
  float inTemp     = NAN;
  float inHumidity = NAN;
  Serial.print(String(TXT_READING_FROM) + " SHT30... ");
  TwoWire I2C_sht = TwoWire(0);
  Adafruit_SHT31 sht30 = Adafruit_SHT31();

  I2C_sht.begin(PIN_I2C_SDA, PIN_I2C_SCL, 100000); // 100kHz
  if (sht30.begin(SHT30_ADDRESS))
  {
    inTemp     = sht30.readTemperature(); // 摄氏度
    inHumidity = sht30.readHumidity();    // %

    // 检查 SHT30 读数是否有效
    if (std::isnan(inTemp) || std::isnan(inHumidity))
    {
      statusStr = "SHT30 " + String(TXT_READ_FAILED);
      Serial.println(statusStr);
    }
    else
    {
      Serial.println(TXT_SUCCESS);
    }
  }
  else
  {
    statusStr = "SHT30 " + String(TXT_NOT_FOUND); // 检查接线
    Serial.println(statusStr);
  }

  String refreshTimeStr;
  getRefreshTimeStr(refreshTimeStr, timeConfigured, &timeInfo);
  String dateStr;
  getDateStr(dateStr, &timeInfo);

  // 全屏刷新渲染
  initDisplay();
  do
  {
    drawCurrentConditions(weather_data.current, weather_data.daily[0],
                          owm_air_pollution, inTemp, inHumidity);
    drawOutlookGraph(weather_data.hourly, weather_data.daily, timeInfo);
    drawForecast(weather_data.daily, timeInfo);
    drawLocationDate(CITY_STRING, dateStr);
#if DISPLAY_ALERTS
    drawAlerts(weather_data.alerts, CITY_STRING, dateStr);
#endif
    drawStatusBar(statusStr, refreshTimeStr, wifiRSSI, batteryVoltage);
  } while (display.nextPage());
  powerOffDisplay();

  // 深度睡眠
  beginDeepSleep(startTime, &timeInfo);
} // end setup

/* 永远不会运行
 */
void loop()
{
} // end loop

