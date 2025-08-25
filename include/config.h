/* esp32-weather-epd 的配置选项声明。
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

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <cstdint>
#include <Arduino.h>


// 若核心未定义板载 LED，引脚默认使用 GPIO2
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

// 电子墨水屏面板
// 支持以下型号：
//   DISP_BW_V2 - 7.5" 黑白 800x480px
//   DISP_3C_B  - 7.5" 红黑白 800x480px
//   DISP_7C_F  - 7.3" 7 色 ACeP 800x480px
//   DISP_BW_V1 - 7.5" 黑白 640x384px
// 取消注释与你的实际面板相对应的宏。
#define DISP_BW_V2
// #define DISP_3C_B
// #define DISP_7C_F
// #define DISP_BW_V1

// 电子墨水驱动板
// 官方仅支持 DESPI-C02 驱动板。
// Waveshare rev2.2 / rev2.3 已弃用，rev2.2 已停产，rev2.3 对比度偏低。
// 取消注释与你的驱动板对应的宏。
#define DRIVER_DESPI_C02
// #define DRIVER_WAVESHARE

// 三色墨水屏强调色
// 在选择三色及以上显示屏时定义第三种颜色。
#if defined(DISP_3C_B) || defined(DISP_7C_F)
  // #define ACCENT_COLOR GxEPD_BLACK
  #define ACCENT_COLOR GxEPD_RED
  // #define ACCENT_COLOR GxEPD_GREEN
  // #define ACCENT_COLOR GxEPD_BLUE
  // #define ACCENT_COLOR GxEPD_YELLOW
  // #define ACCENT_COLOR GxEPD_ORANGE
#endif

// 语言设置
// 若列表中没有你的语言，可复制 src/locales 下的文件进行修改，欢迎提交 PR。
//   语言（地区）                    代码
//   德语（德国）                    de_DE
//   英语（英国）                    en_GB
//   英语（美国）                    en_US
//   爱沙尼亚语（爱沙尼亚）          et_EE
//   芬兰语（芬兰）                  fi_FI
//   法语（法国）                    fr_FR
//   意大利语（意大利）              it_IT
//   荷兰语（比利时）                nl_BE
//   葡萄牙语（巴西）                pt_BR
#define LOCALE en_US

// 单位设置
// 每种度量类型只能定义一个宏。

// 温度单位
//   公制：摄氏度
//   英制：华氏度
// #define UNITS_TEMP_KELVIN
// #define UNITS_TEMP_CELSIUS
#define UNITS_TEMP_FAHRENHEIT

// 风速单位
//   公制：公里/小时
//   英制：英里/小时
// #define UNITS_SPEED_METERSPERSECOND
// #define UNITS_SPEED_FEETPERSECOND
// #define UNITS_SPEED_KILOMETERSPERHOUR
#define UNITS_SPEED_MILESPERHOUR
// #define UNITS_SPEED_KNOTS
// #define UNITS_SPEED_BEAUFORT

// 气压单位
//   本项目已放弃气压显示，以下宏保留为空
// #define UNITS_PRES_HECTOPASCALS

// 能见度单位
//   公制：公里
//   英制：英里
// #define UNITS_DIST_KILOMETERS
#define UNITS_DIST_MILES

// 降水量单位（每小时）
// 可选择降水概率 PoP 或每小时降水量。
//   公制：毫米
//   英制：英寸
#define UNITS_HOURLY_PRECIP_POP
// #define UNITS_HOURLY_PRECIP_MILLIMETERS
// #define UNITS_HOURLY_PRECIP_CENTIMETERS
// #define UNITS_HOURLY_PRECIP_INCHES

// 降水量单位（每日）
// 可选择降水概率 PoP 或每日降水量。
//   公制：毫米
//   英制：英寸
// #define UNITS_DAILY_PRECIP_POP
// #define UNITS_DAILY_PRECIP_MILLIMETERS
// #define UNITS_DAILY_PRECIP_CENTIMETERS
#define UNITS_DAILY_PRECIP_INCHES

// 传输协议选项
// HTTP
//   不加密，易被窃听与篡改，但功耗最低。
// HTTPS_NO_CERT_VERIF
//   HTTPS 但不校验证书，虽加密但可能遭遇中间人攻击。
// HTTPS_WITH_CERT_VERIF
//   HTTPS 并校验证书，安全性最高，可确认服务器身份。
//
//   使用证书校验需要在证书到期后更新 cert.h 并重新烧录。
//   运行 cert.py 可生成新的 cert.h。
//   目前 api.openweathermap.org 证书有效期至 2030-12-31 23:59:59。
// （仅取消注释一个）
// #define USE_HTTP
// #define USE_HTTPS_NO_CERT_VERIF
#define USE_HTTPS_WITH_CERT_VERIF

// 风向指示方式
// 可选择箭头、数字或罗盘方位表示，可同时与数字或方位组合。
//
//   精度级别                   数量   误差    例子
//   基本方向 (Cardinal)        4   ±45.000°  E
//   次方位 (Intercardinal)     8   ±22.500°  NE
//   二级方位 (Secondary)      16   ±11.250°  NNE
//   三级方位 (Tertiary)       32   ±5.625°   NbE
#define WIND_INDICATOR_ARROW
// #define WIND_INDICATOR_NUMBER
// #define WIND_INDICATOR_CPN_CARDINAL
// #define WIND_INDICATOR_CPN_INTERCARDINAL
// #define WIND_INDICATOR_CPN_SECONDARY_INTERCARDINAL
// #define WIND_INDICATOR_CPN_TERTIARY_INTERCARDINAL
// #define WIND_INDICATOR_NONE

// 风向图标精度
// 温度图左侧的风向图标可表示不同精度，精度越高需要的存储越多。
// 360 个图标约占用 25kB 闪存。
//
//   精度级别                   数量  误差    存储
//   基本方向                   4   ±45.000°   288B  E
//   次方位                     8   ±22.500°   576B  NE
//   二级方位                  16   ±11.250° 1,152B  NNE
//   三级方位                  32   ±5.625°  2,304B  NbE
//   (360)                    360  ±0.500°  25,920B  1°
// 取消注释你希望的精度级别。
// #define WIND_ICONS_CARDINAL
// #define WIND_ICONS_INTERCARDINAL
#define WIND_ICONS_SECONDARY_INTERCARDINAL
// #define WIND_ICONS_TERTIARY_INTERCARDINAL
// #define WIND_ICONS_360

// 字体
// 项目内置多种开源字体，可通过选择对应头文件切换。
//
//   字体名          头文件                   字体家族        许可证
//   FreeMono       FreeMono.h               GNU FreeFont    GNU GPL v3.0
//   FreeSans       FreeSans.h               GNU FreeFont    GNU GPL v3.0
//   FreeSerif      FreeSerif.h              GNU FreeFont    GNU GPL v3.0
//   Lato           Lato_Regular.h           Lato            SIL OFL v1.1
//   Montserrat     Montserrat_Regular.h     Montserrat      SIL OFL v1.1
//   Open Sans      OpenSans_Regular.h       Open Sans       SIL OFL v1.1
//   Poppins        Poppins_Regular.h        Poppins         SIL OFL v1.1
//   Quicksand      Quicksand_Regular.h      Quicksand       SIL OFL v1.1
//   Raleway        Raleway_Regular.h        Raleway         SIL OFL v1.1
//   Roboto         Roboto_Regular.h         Roboto          Apache v2.0
//   Roboto Mono    RobotoMono_Regular.h     Roboto Mono     Apache v2.0
//   Roboto Slab    RobotoSlab_Regular.h     Roboto Slab     Apache v2.0
//   Ubuntu         Ubuntu_R.h               Ubuntu          UFL v1.0
//   Ubuntu Mono    UbuntuMono_R.h           Ubuntu          UFL v1.0
//
// 如需添加新字体请参阅 fonts/README。
//
// 注意：
//   屏幕布局以 GNU FreeSans 字体为基础，替换其他字体可能导致间距异常。
#define FONT_HEADER "fonts/FreeSans.h"

// 每日降水显示
// Hi|Lo 下方的降水量可以按以下选项配置：
//   0 : 禁用（始终隐藏）
//   1 : 启用（始终显示）
//   2 : 智能（仅在预测有降水时显示）
#define DISPLAY_DAILY_PRECIP 2

// 小时天气图标
// 在温度/降水图上显示小时图标，绘制在 x 轴刻度处。
//   0 : 禁用
//   1 : 启用
#define DISPLAY_HOURLY_ICONS 1

// 天气警报
//   各国警报系统格式差异巨大，OpenWeatherMap 仅提供英文警报。
//   若不希望显示，可将 DISPLAY_ALERTS 设为 0。
#define DISPLAY_ALERTS 1

// 状态栏附加信息
//   设置为 1 以显示额外信息。
#define STATUS_BAR_EXTRAS_BAT_VOLTAGE 0
#define STATUS_BAR_EXTRAS_WIFI_RSSI   0

// 电池监测
//   显示屏可带或不带电池供电，低功耗策略在 config.cpp 中设置。
//   如需禁用电池监测，将该宏设为 0。
#define BATTERY_MONITORING 1

// NON-VOLATILE STORAGE (NVS) NAMESPACE
#define NVS_NAMESPACE "weather_epd"

// 调试级别
//   控制串口输出信息量：
//   level 0: 基本状态（默认）
//   level 1: 更多调试信息
//   level 2: 打印 API 返回数据
#define DEBUG_LEVEL 0

// 以下常量在 "config.cpp" 中定义
extern const uint8_t PIN_BAT_ADC;
extern const uint8_t PIN_EPD_BUSY;
extern const uint8_t PIN_EPD_CS;
extern const uint8_t PIN_EPD_RST;
extern const uint8_t PIN_EPD_DC;
extern const uint8_t PIN_EPD_SCK;
extern const uint8_t PIN_EPD_MISO;
extern const uint8_t PIN_EPD_MOSI;
extern const uint8_t PIN_EPD_PWR;
extern const uint8_t PIN_I2C_SDA;
extern const uint8_t PIN_I2C_SCL;
extern const uint8_t SHT30_ADDRESS;
extern const uint8_t RTC_ADDRESS;
extern const char *WIFI_SSID;
extern const char *WIFI_PASSWORD;
extern const unsigned long WIFI_TIMEOUT;
extern const unsigned HTTP_CLIENT_TCP_TIMEOUT;
extern const String CMA_PID;
extern const String CMA_KEY;
extern const String CMA_PROVINCE;
extern const String CMA_CITY;
extern const String CMA_PLACE;
extern const String CMA_ENDPOINT;
extern const String LAT;
extern const String LON;
extern const String CITY_STRING;
extern const char *TIMEZONE;
extern const char *TIME_FORMAT;
extern const char *HOUR_FORMAT;
extern const char *DATE_FORMAT;
extern const char *REFRESH_TIME_FORMAT;
extern const char *NTP_SERVER_1;
extern const char *NTP_SERVER_2;
extern const unsigned long NTP_TIMEOUT;
extern const int SLEEP_DURATION;
extern const int BED_TIME;
extern const int WAKE_TIME;
extern const int HOURLY_GRAPH_MAX;
extern const uint32_t WARN_BATTERY_VOLTAGE;
extern const uint32_t LOW_BATTERY_VOLTAGE;
extern const uint32_t VERY_LOW_BATTERY_VOLTAGE;
extern const uint32_t CRIT_LOW_BATTERY_VOLTAGE;
extern const unsigned long LOW_BATTERY_SLEEP_INTERVAL;
extern const unsigned long VERY_LOW_BATTERY_SLEEP_INTERVAL;
extern const uint32_t MAX_BATTERY_VOLTAGE;
extern const uint32_t MIN_BATTERY_VOLTAGE;

// CONFIG VALIDATION - DO NOT MODIFY
#if !(  defined(DISP_BW_V2)  \
      ^ defined(DISP_3C_B)   \
      ^ defined(DISP_7C_F)   \
      ^ defined(DISP_BW_V1))
  #error Invalid configuration. Exactly one display panel must be selected.
#endif
#if !(  defined(DRIVER_WAVESHARE) \
      ^ defined(DRIVER_DESPI_C02))
  #error Invalid configuration. Exactly one driver board must be selected.
#endif
#if !(defined(LOCALE))
  #error Invalid configuration. Locale not selected.
#endif
#if !(  defined(UNITS_TEMP_KELVIN)      \
      ^ defined(UNITS_TEMP_CELSIUS)     \
      ^ defined(UNITS_TEMP_FAHRENHEIT))
  #error Invalid configuration. Exactly one temperature unit must be selected.
#endif
#if !(  defined(UNITS_SPEED_METERSPERSECOND)   \
      ^ defined(UNITS_SPEED_FEETPERSECOND)     \
      ^ defined(UNITS_SPEED_KILOMETERSPERHOUR) \
      ^ defined(UNITS_SPEED_MILESPERHOUR)      \
      ^ defined(UNITS_SPEED_KNOTS)             \
      ^ defined(UNITS_SPEED_BEAUFORT))
  #error Invalid configuration. Exactly one wind speed unit must be selected.
#endif
// 已移除气压功能，上述宏检查不再需要
#if !(  defined(UNITS_DIST_KILOMETERS) \
      ^ defined(UNITS_DIST_MILES))
  #error Invalid configuration. Exactly one distance unit must be selected.
#endif
#if !(  defined(UNITS_HOURLY_PRECIP_POP)         \
      ^ defined(UNITS_HOURLY_PRECIP_MILLIMETERS) \
      ^ defined(UNITS_HOURLY_PRECIP_CENTIMETERS) \
      ^ defined(UNITS_HOURLY_PRECIP_INCHES))
  #error Invalid configuration. Exactly one houly precipitation measurement must be selected.
#endif
#if !(  defined(UNITS_DAILY_PRECIP_POP)         \
      ^ defined(UNITS_DAILY_PRECIP_MILLIMETERS) \
      ^ defined(UNITS_DAILY_PRECIP_CENTIMETERS) \
      ^ defined(UNITS_DAILY_PRECIP_INCHES))
  #error Invalid configuration. Exactly one daily precipitation measurement must be selected.
#endif
#if !(  defined(USE_HTTP)                   \
      ^ defined(USE_HTTPS_NO_CERT_VERIF)    \
      ^ defined(USE_HTTPS_WITH_CERT_VERIF))
  #error Invalid configuration. Exactly one HTTP mode must be selected.
#endif
#if !(  defined(WIND_INDICATOR_ARROW)                         \
      || (                                                    \
          defined(WIND_INDICATOR_NUMBER)                      \
        ^ defined(WIND_INDICATOR_CPN_CARDINAL)                \
        ^ defined(WIND_INDICATOR_CPN_INTERCARDINAL)           \
        ^ defined(WIND_INDICATOR_CPN_SECONDARY_INTERCARDINAL) \
        ^ defined(WIND_INDICATOR_CPN_TERTIARY_INTERCARDINAL)  \
      )                                                       \
      ^ defined(WIND_INDICATOR_NONE))
  #error Invalid configuration. Illegal selction of wind indicator(s).
#endif
#if defined(WIND_INDICATOR_ARROW)                   \
 && !(  defined(WIND_ICONS_CARDINAL)                \
      ^ defined(WIND_ICONS_INTERCARDINAL)           \
      ^ defined(WIND_ICONS_SECONDARY_INTERCARDINAL) \
      ^ defined(WIND_ICONS_TERTIARY_INTERCARDINAL)  \
      ^ defined(WIND_ICONS_360))
  #error Invalid configuration. Exactly one wind direction icon precision level must be selected.
#endif
#if !(defined(FONT_HEADER))
  #error Invalid configuration. Font not selected.
#endif
#if !(defined(DISPLAY_DAILY_PRECIP))
  #error Invalid configuration. DISPLAY_DAILY_PRECIP not defined.
#endif
#if !(defined(DISPLAY_HOURLY_ICONS))
  #error Invalid configuration. DISPLAY_HOURLY_ICONS not defined.
#endif
#if !(defined(DISPLAY_ALERTS))
  #error Invalid configuration. DISPLAY_ALERTS not defined.
#endif
#if !(defined(BATTERY_MONITORING))
  #error Invalid configuration. BATTERY_MONITORING not defined.
#endif
#if !(defined(DEBUG_LEVEL))
  #error Invalid configuration. DEBUG_LEVEL not defined.
#endif

#endif
