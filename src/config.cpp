/* esp32-weather-epd 的配置选项
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
#include "config.h"

// 引脚定义
// 下方配置适用于本项目官方接线图，使用 FireBeetle 2 ESP32-E 微控制器板。
//
// 注意：LED_BUILTIN 引脚将被禁用以降低功耗。请参考你的开发板引脚图，确保避免使用具有该共享功能的引脚。
//
// 用于测量电池电压的 ADC 引脚
const uint8_t PIN_BAT_ADC  = A2; // micro-usb firebeetle 用 A0
// E-Paper 驱动板引脚
const uint8_t PIN_EPD_BUSY = 14;  // EPD_BUSY
const uint8_t PIN_EPD_CS   = 13;  // EPD_CS
const uint8_t PIN_EPD_RST  = 21;  // EPD_RST
const uint8_t PIN_EPD_DC   = 22;  // EPD_DC
const uint8_t PIN_EPD_SCK  = 18;  // EPD_CLK
const uint8_t PIN_EPD_MISO = 19;  // SPI MISO (未使用)
const uint8_t PIN_EPD_MOSI = 23;  // EPD_DIN
const uint8_t PIN_EPD_PWR  = 26;  // 电源控制（可直接 3.3V）
// I2C 引脚（SHT30 温湿度传感器与 RTC 共用）
const uint8_t PIN_I2C_SDA = 17;
const uint8_t PIN_I2C_SCL = 16;
const uint8_t SHT30_ADDRESS = 0x44; // SHT30 默认地址
const uint8_t RTC_ADDRESS   = 0x51; // BL8025C/PCF8563 地址


// WIFI
const char *WIFI_SSID     = "ssid";
const char *WIFI_PASSWORD = "password";
const unsigned long WIFI_TIMEOUT = 10000; // 毫秒，WiFi 连接超时时间

// HTTP
// 下列错误通常是由于 http 客户端 tcp 超时不足导致：
//   -1   连接被拒绝
//   -11  读取超时
//   -258 反序列化输入不完整
const unsigned HTTP_CLIENT_TCP_TIMEOUT = 10000; // 毫秒

// 中国气象台 API
// 参考：https://cn.apihz.cn/api/tianqi/tqyb.php
const String CMA_PID      = "your_pid";        // 用户 PID
const String CMA_KEY      = "your_key";        // 用户 KEY
const String CMA_PROVINCE = "省份";           // 省
const String CMA_CITY     = "城市";           // 市
const String CMA_PLACE    = "区县";           // 区/县
const String CMA_ENDPOINT = "cn.apihz.cn";    // 接口域名

// 位置
// 设置你的纬度和经度。
// （用于向 OpenWeatherMap API 请求天气数据）
const String LAT = "40.7128";
const String LON = "-74.0060";
// 城市名称，将显示在屏幕右上角。
const String CITY_STRING = "New York";

// 时间
// 时区列表见
// https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
const char *TIMEZONE = "EST5EDT,M3.2.0,M11.1.0";
// 显示日出/日落时间时使用的时间格式（最多 11 个字符）。
// 更多格式化信息见
// https://man7.org/linux/man-pages/man3/strftime.3.html
// const char *TIME_FORMAT = "%l:%M%P"; // 12 小时制，如 1:23am  11:00pm
const char *TIME_FORMAT = "%H:%M";   // 24 小时制，如 01:23   23:00
// 显示坐标轴标签时使用的时间格式（最多 11 个字符）。
// 更多格式化信息见
// https://man7.org/linux/man-pages/man3/strftime.3.html
// const char *HOUR_FORMAT = "%l%P"; // 12 小时制，如 1am  11pm
const char *HOUR_FORMAT = "%H";      // 24 小时制，如 01   23
// 显示右上角日期时使用的日期格式。
// 更多格式化信息见
// https://man7.org/linux/man-pages/man3/strftime.3.html
const char *DATE_FORMAT = "%a, %B %e"; // 如：Sat, January 1
// 显示屏幕底部最后刷新时间时使用的日期/时间格式。
// 更多格式化信息见
// https://man7.org/linux/man-pages/man3/strftime.3.html
const char *REFRESH_TIME_FORMAT = "%x %H:%M";
// NTP_SERVER_1 为主时间服务器，NTP_SERVER_2 为备用。
// pool.ntp.org 会自动选择离你最近的 NTP 服务器。
const char *NTP_SERVER_1 = "pool.ntp.org";
const char *NTP_SERVER_2 = "time.nist.gov";
// 若遇到 'Failed To Fetch The Time' 错误，可尝试增加 NTP_TIMEOUT 或选择更近/延迟更低的时间服务器。
const unsigned long NTP_TIMEOUT = 20000; // 毫秒
// 睡眠时长（分钟），即 esp32 唤醒更新的频率。
// 对齐到最近的分钟边界。
// 例如，设为 30（分钟），则显示将在每小时的 00 或 30 分钟更新。（范围：[2-1440]）
// 注意：OpenWeatherMap 模型每 10 分钟更新一次，故无需更频繁刷新。
const int SLEEP_DURATION = 30; // 分钟
// 睡眠时段省电设置。
// 若 BED_TIME == WAKE_TIME，则该省电功能关闭。
// （范围：[0-23]）
const int BED_TIME  = 00; // 最后一次更新在 00:00（午夜），直到 WAKE_TIME。
const int WAKE_TIME = 06; // 睡眠后首次更新的小时，06:00。
// 注意，SLEEP_DURATION 的分钟对齐从 WAKE_TIME 开始，即使省电功能关闭。
// 例如，WAKE_TIME = 00（午夜），SLEEP_DURATION = 120，则显示将在 00:00、02:00、04:00... 更新，直到 BED_TIME。
// 若希望每天仅刷新一次，可设 SLEEP_DURATION = 1440，并通过 BED_TIME 和 WAKE_TIME 设置每天刷新时间。

// 小时趋势图
// 趋势图显示的小时数（范围：[8-48]）
const int HOURLY_GRAPH_MAX = 24;

// 电池
// 为保护电池，当电压低于 LOW_BATTERY_VOLTAGE 时，显示将停止更新，直到电池充电。
// ESP32 将进入深度睡眠（< 11μA），定时唤醒检测电压。
// 若电压降至 CRIT_LOW_BATTERY_VOLTAGE，esp32 将休眠，需手动按下复位（RST）按钮恢复运行。
const uint32_t WARN_BATTERY_VOLTAGE     = 3535; // 毫伏，约 20%
const uint32_t LOW_BATTERY_VOLTAGE      = 3462; // 毫伏，约 10%
const uint32_t VERY_LOW_BATTERY_VOLTAGE = 3442; // 毫伏，约 8%
const uint32_t CRIT_LOW_BATTERY_VOLTAGE = 3404; // 毫伏，约 5%
const unsigned long LOW_BATTERY_SLEEP_INTERVAL      = 30;  // 分钟
const unsigned long VERY_LOW_BATTERY_SLEEP_INTERVAL = 120; // 分钟
// 电池电压计算基于典型 3.7V 锂电池。
const uint32_t MAX_BATTERY_VOLTAGE = 4200; // 毫伏
const uint32_t MIN_BATTERY_VOLTAGE = 3000; // 毫伏

// 其他选项见 config.h
// E-PAPER 面板
// 地区设置
// 单位
// 风向图标精度
// 字体
// 警报
// 电池