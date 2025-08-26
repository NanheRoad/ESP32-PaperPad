/* 显示相关工具函数 */
#include <cmath>
#include <Arduino.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>

#include "_locale.h"
#include "_strftime.h"
#include "display_utils.h"
#include "config.h"
#include "icons/icons.h"

/* 读取电池电压，返回毫伏 */
uint32_t readBatteryVoltage()
{
  esp_adc_cal_characteristics_t adc_chars;
  esp_adc_cal_value_t val_type __attribute__((unused));
  adc_power_acquire();
  uint16_t adc_val = analogRead(PIN_BAT_ADC);
  adc_power_release();

  // 使用 eFuse 校准以获得更准确的读数，11dB 衰减可测量 0~3.3V
  val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_11db,
                                      ADC_WIDTH_BIT_12, 1100, &adc_chars);
  uint32_t batteryVoltage = esp_adc_cal_raw_to_voltage(adc_val, &adc_chars);
  // 根据外部分压电阻，电压需乘以 2
  batteryVoltage *= 2;
  return batteryVoltage;
}

/* 根据电压估算电量百分比 */
uint32_t calcBatPercent(uint32_t v, uint32_t minv, uint32_t maxv)
{
  uint32_t p = 105 - (105 / (1 + pow(1.724 * (v - minv)/(maxv - minv), 5.5)));
  return p >= 100 ? 100 : p;
}

/* 根据电量百分比获取 24x24 电池图标 */
const uint8_t *getBatBitmap24(uint32_t batPercent)
{
  if (batPercent >= 93) { return battery_full_90deg_24x24; }
  else if (batPercent >= 79) { return battery_6_bar_90deg_24x24; }
  else if (batPercent >= 65) { return battery_5_bar_90deg_24x24; }
  else if (batPercent >= 50) { return battery_4_bar_90deg_24x24; }
  else if (batPercent >= 36) { return battery_3_bar_90deg_24x24; }
  else if (batPercent >= 22) { return battery_2_bar_90deg_24x24; }
  else if (batPercent >= 8)  { return battery_1_bar_90deg_24x24; }
  else                      { return battery_0_bar_90deg_24x24; }
}

/* 获取当前日期字符串 */
void getDateStr(String &s, tm *timeInfo)
{
  char buf[48] = {};
  _strftime(buf, sizeof(buf), DATE_FORMAT, timeInfo);
  s = buf;
  s.replace("  ", " ");
}

/* 获取刷新时间字符串 */
void getRefreshTimeStr(String &s, bool timeSuccess, tm *timeInfo)
{
  if (!timeSuccess) {
    s = TXT_UNKNOWN;
    return;
  }
  char buf[48] = {};
  _strftime(buf, sizeof(buf), REFRESH_TIME_FORMAT, timeInfo);
  s = buf;
  s.replace("  ", " ");
}

/* 根据 RSSI 返回 WiFi 描述 */
const char *getWiFidesc(int rssi)
{
  if (rssi == 0) { return TXT_WIFI_NO_CONNECTION; }
  if (rssi >= -50) { return TXT_WIFI_EXCELLENT; }
  if (rssi >= -60) { return TXT_WIFI_GOOD; }
  if (rssi >= -67) { return TXT_WIFI_FAIR; }
  return TXT_WIFI_WEAK;
}

/* 根据 RSSI 获取 16x16 WiFi 图标 */
const uint8_t *getWiFiBitmap16(int rssi)
{
  // 当前资源仅提供一个 16x16 WiFi 图标，占位返回
  return wifi_x_16x16;
}

/* HTTP 状态码对应描述 */
const char *getHttpResponsePhrase(int code)
{
  switch (code)
  {
    case 200: return "成功";
    case 400: return "错误请求";
    case 401: return "未授权";
    case 403: return "拒绝访问";
    case 404: return "未找到";
    default:  return "HTTP 错误";
  }
}

/* WiFi 连接状态描述 */
const char *getWifiStatusPhrase(wl_status_t status)
{
  switch (status)
  {
    case WL_IDLE_STATUS:     return "空闲";
    case WL_NO_SSID_AVAIL:   return "无 SSID";
    case WL_CONNECT_FAILED:  return "连接失败";
    case WL_CONNECTED:       return "已连接";
    case WL_DISCONNECTED:    return "已断开";
    default:                 return "未知";
  }
}

/* 打印堆内存使用情况 */
void printHeapUsage()
{
  Serial.println("[调试] 堆大小       : " + String(ESP.getHeapSize()) + " B");
  Serial.println("[调试] 可用堆       : " + String(ESP.getFreeHeap()) + " B");
  Serial.println("[调试] 最小可用堆   : " + String(ESP.getMinFreeHeap()) + " B");
  Serial.println("[调试] 最大可分配   : " + String(ESP.getMaxAllocHeap()) + " B");
}

/* 关闭板载 LED 以降低功耗 */
void disableBuiltinLED()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

