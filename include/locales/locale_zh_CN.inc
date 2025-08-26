/* Simplified Chinese locale data for esp32-weather-epd.
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

#include "_locale.h"
#include <vector>
#include <Arduino.h>

// LC_TIME
// locale-based information,
// see https://man7.org/linux/man-pages/man3/nl_langinfo.3.html for more info.
// Note to Translators:
//   The LC_TIME definitions are included in the localedata/locales directory of
//   the glibc source tree, in files named after the locale codes and encoding
//   schemes. For example, the en_US locale data is stored in the
//   localedata/locales/en_US file, which contains the definitions for date and
//   time formats, month and day names, and other time-related settings for the
//   LC_TIME category.
//   https://www.gnu.org/software/libc/sources.html
// D_T_FMT                 string for formatting date and time
const char *LC_D_T_FMT     = "%Y年%m月%d日 %H:%M:%S %Z";
// LC_D_FMT                date format string
const char *LC_D_FMT       = "%Y-%m-%d";
// T_FMT                   time format string
const char *LC_T_FMT       = "%H:%M:%S";
// T_FMT_AMPM              a.m. or p.m. time format string
const char *LC_T_FMT_AMPM  = "%I:%M:%S %p";
// AM_STR                  Ante Meridian affix
const char *LC_AM_STR      = "上午";
// PM_STR                  Post Meridian affix
const char *LC_PM_STR      = "下午";
// DAY_{1-7}               name of the n-th day of the week
const char *LC_DAY[7]      = {"星期日", "星期一", "星期二", "星期三",
                              "星期四", "星期五", "星期六"};
// ABDAY_{1-7}             abbreviated name of the n-th day of the week
const char *LC_ABDAY[7]    = {"周日", "周一", "周二", "周三", "周四", "周五", "周六"};
// MON_{1-12}              name of the n-th month of the year
const char *LC_MON[12]     = {"一月",  "二月",  "三月",  "四月",
                               "五月",  "六月",  "七月",  "八月",
                               "九月",  "十月",  "十一月", "十二月"};
// ABMON_{1-12}            abbreviated name of the n-th month of the year
const char *LC_ABMON[12]   = {"1月", "2月", "3月", "4月", "5月", "6月",
                              "7月", "8月", "9月", "10月", "11月", "12月"};
// ERA                     era description segments
const char *LC_ERA         = "";
// ERA_D_FMT               era date format string
const char *LC_ERA_D_FMT   = "";
// ERA_D_T_FMT             era date and time format string
const char *LC_ERA_D_T_FMT = "";
// ERA_T_FMT               era time format string
const char *LC_ERA_T_FMT   = "";

// OWM LANGUAGE
// For full list of languages, see
// https://openweathermap.org/api/one-call-api#multi
// Note: "[only] The contents of the 'description' field will be translated."
const String OWM_LANG = "zh_cn";

// CURRENT CONDITIONS
const char *TXT_FEELS_LIKE         = "体感";
const char *TXT_SUNRISE            = "日出";
const char *TXT_SUNSET             = "日落";
const char *TXT_WIND               = "风";
const char *TXT_HUMIDITY           = "湿度";
const char *TXT_UV_INDEX           = "紫外指数";
const char *TXT_PRESSURE           = "气压";
const char *TXT_AIR_QUALITY        = "空气质量";
const char *TXT_AIR_POLLUTION      = "空气污染";
const char *TXT_VISIBILITY         = "能见度";
const char *TXT_INDOOR_TEMPERATURE = "室内温度";
const char *TXT_INDOOR_HUMIDITY    = "室内湿度";

// UV INDEX
const char *TXT_UV_LOW       = "低";
const char *TXT_UV_MODERATE  = "中等";
const char *TXT_UV_HIGH      = "高";
const char *TXT_UV_VERY_HIGH = "很高";
const char *TXT_UV_EXTREME   = "极高";

// WIFI
const char *TXT_WIFI_EXCELLENT     = "极好";
const char *TXT_WIFI_GOOD          = "良好";
const char *TXT_WIFI_FAIR          = "一般";
const char *TXT_WIFI_WEAK          = "较弱";
const char *TXT_WIFI_NO_CONNECTION = "未连接";

// UNIT SYMBOLS - TEMPERATURE
const char *TXT_UNITS_TEMP_KELVIN     = "K";
const char *TXT_UNITS_TEMP_CELSIUS    = "\260C";
const char *TXT_UNITS_TEMP_FAHRENHEIT = "\260F";
// UNIT SYMBOLS - WIND SPEED
const char *TXT_UNITS_SPEED_METERSPERSECOND   = "m/s";
const char *TXT_UNITS_SPEED_FEETPERSECOND     = "ft/s";
const char *TXT_UNITS_SPEED_KILOMETERSPERHOUR = "km/h";
const char *TXT_UNITS_SPEED_MILESPERHOUR      = "mph";
const char *TXT_UNITS_SPEED_KNOTS             = "kt";
const char *TXT_UNITS_SPEED_BEAUFORT          = "";
// UNIT SYMBOLS - PRESSURE
const char *TXT_UNITS_PRES_HECTOPASCALS             = "hPa";
const char *TXT_UNITS_PRES_PASCALS                  = "Pa";
const char *TXT_UNITS_PRES_MILLIMETERSOFMERCURY     = "mmHg";
const char *TXT_UNITS_PRES_INCHESOFMERCURY          = "inHg";
const char *TXT_UNITS_PRES_MILLIBARS                = "mbar";
const char *TXT_UNITS_PRES_ATMOSPHERES              = "atm";
const char *TXT_UNITS_PRES_GRAMSPERSQUARECENTIMETER = "g/cm\262";
const char *TXT_UNITS_PRES_POUNDSPERSQUAREINCH      = "lb/in\262";
// UNITS SYMBOLS - VISIBILITY DISTANCE
const char *TXT_UNITS_DIST_KILOMETERS = "km";
const char *TXT_UNITS_DIST_MILES      = "mi";
// UNITS SYMBOLS - PRECIPITATION
const char *TXT_UNITS_PRECIP_MILLIMETERS = "mm";
const char *TXT_UNITS_PRECIP_CENTIMETERS = "cm";
const char *TXT_UNITS_PRECIP_INCHES      = "in";

// MISCELLANEOUS MESSAGES
// Title Case
const char *TXT_LOW_BATTERY = "电量低";
const char *TXT_NETWORK_NOT_AVAILABLE = "网络不可用";
const char *TXT_TIME_SYNCHRONIZATION_FAILED = "时间同步失败";
const char *TXT_WIFI_CONNECTION_FAILED = "WiFi 连接失败";
// First Word Capitalized
const char *TXT_ATTEMPTING_HTTP_REQ = "正在发送 HTTP 请求";
const char *TXT_AWAKE_FOR = "已唤醒";
const char *TXT_BATTERY_VOLTAGE = "电池电压";
const char *TXT_CONNECTING_TO = "正在连接";
const char *TXT_COULD_NOT_CONNECT_TO = "无法连接到";
const char *TXT_ENTERING_DEEP_SLEEP_FOR = "进入深度睡眠";
const char *TXT_READING_FROM = "读取自";
const char *TXT_FAILED  = "失败";
const char *TXT_SUCCESS = "成功";
const char *TXT_UNKNOWN = "未知";
// All Lowercase
const char *TXT_NOT_FOUND = "未找到";
const char *TXT_READ_FAILED = "读取失败";
// Complete Sentences
const char *TXT_FAILED_TO_GET_TIME = "获取时间失败!";
const char *TXT_HIBERNATING_INDEFINITELY_NOTICE = "无唤醒时间，进入休眠!";
const char *TXT_REFERENCING_OLDER_TIME_NOTICE = "深度睡眠前时间同步失败，将使用旧时间。";
const char *TXT_WAITING_FOR_SNTP = "正在等待 SNTP 同步。";
const char *TXT_LOW_BATTERY_VOLTAGE = "电池电压低!";
const char *TXT_VERY_LOW_BATTERY_VOLTAGE = "电池电压很低!";
const char *TXT_CRIT_LOW_BATTERY_VOLTAGE = "电池电压严重偏低!";

// ALERTS
// The display can show up to 2 alerts, but alerts can be unpredictable in
// severity and number. If more than 2 alerts are active, the esp32 will attempt
// to interpret the urgency of each alert and prefer to display the most urgent
// and recently issued alerts of each event type. Depending on your region
// different keywords are used to convey the level of urgency.
//
// A vector array is used to store these keywords. Urgency is ranked from low to
// high where the first index of the vector is the least urgent keyword and the
// last index is the most urgent keyword. Expected as all lowercase.
//
// Note to Translators:
//   OpenWeatherMap returns alerts in English regardless of the OWM LANGUAGE
//   option or territory. For this reason it is preferred that you do not
//   translate text related to Alerts. Understandably, it may be undesirable to
//   see English alerts in territories where English is uncommon. To satisfy
//   this, users should set the DISPLAY_ALERTS macro to 0 in config.h.
//
// Here are a few examples, uncomment the array for your region (or create your
// own).
const std::vector<String> ALERT_URGENCY = {"outlook", "statement", "watch", "advisory", "warning", "emergency"}; // US National Weather Service
// const std::vector<String> ALERT_URGENCY = {"yellow", "amber", "red"};                 // United Kingdom's national weather service (MET Office)
// const std::vector<String> ALERT_URGENCY = {"minor", "moderate", "severe", "extreme"}; // METEO
// const std::vector<String> ALERT_URGENCY = {}; // Disable urgency interpretation (algorithm will fallback to only prefer the most recently issued alerts)

// ALERT TERMINOLOGY
// Weather terminology associated with each alert icon
const std::vector<String> TERM_SMOG =
    {"smog"};
const std::vector<String> TERM_SMOKE =
    {"smoke"};
const std::vector<String> TERM_FOG =
    {"fog", "haar"};
const std::vector<String> TERM_METEOR =
    {"meteor", "asteroid"};
const std::vector<String> TERM_NUCLEAR =
    {"nuclear", "ionizing radiation"};
const std::vector<String> TERM_BIOHAZARD =
    {"biohazard", "biological hazard"};
const std::vector<String> TERM_EARTHQUAKE =
    {"earthquake"};
const std::vector<String> TERM_FIRE =
    {"fire", "red flag"};
const std::vector<String> TERM_HEAT =
    {"heat"};
const std::vector<String> TERM_WINTER =
    {"blizzard", "winter", "ice", "icy", "snow", "sleet", "cold",
     "freezing rain", "wind chill", "freeze", "frost", "hail"};
const std::vector<String> TERM_TSUNAMI =
    {"tsunami", "surf"};
const std::vector<String> TERM_LIGHTNING =
    {"thunderstorm", "storm cell", "pulse storm", "squall line", "supercell",
     "lightning"};
const std::vector<String> TERM_SANDSTORM =
    {"sandstorm", "blowing dust", "dust storm"};
const std::vector<String> TERM_FLOOD =
    {"flood", "storm surge", "seiche", "swell", "high seas", "high tides",
     "tidal surge", "hydrologic"};
const std::vector<String> TERM_VOLCANO =
    {"volcanic", "ash", "volcano", "eruption"};
const std::vector<String> TERM_AIR_QUALITY =
    {"air", "stagnation", "pollution"};
const std::vector<String> TERM_TORNADO =
    {"tornado"};
const std::vector<String> TERM_SMALL_CRAFT_ADVISORY =
    {"small craft", "wind advisory"};
const std::vector<String> TERM_GALE_WARNING =
    {"gale"};
const std::vector<String> TERM_STORM_WARNING =
    {"storm warning"};
const std::vector<String> TERM_HURRICANE_WARNING =
    {"hurricane force wind", "extreme wind", "high wind"};
const std::vector<String> TERM_HURRICANE =
    {"hurricane", "tropical storm", "typhoon", "cyclone"};
const std::vector<String> TERM_DUST =
    {"dust", "sand"};
const std::vector<String> TERM_STRONG_WIND =
    {"wind"};

// AIR QUALITY INDEX
extern "C" {
// Seemingly every country uses a different scale for Air Quality Index (AQI).
// I have written a library to calculate many of the most popular AQI scales.
// Feel free to request the addition of a new AQI scale by opening an Issue.
// https://github.com/lmarzen/pollutant-concentration-to-aqi
// Please choose this region's preferred AQI scale.
//   AUSTRALIA_AQI        CANADA_AQHI          EUROPEAN_UNION_CAQI
//   HONG_KONG_AQHI       INDIA_AQI            CHINA_AQI
//   SINGAPORE_PSI        SOUTH_KOREA_CAI      UNITED_KINGDOM_DAQI
//   UNITED_STATES_AQI
const aqi_scale_t AQI_SCALE = UNITED_STATES_AQI;

const char *AUSTRALIA_AQI_TXT[6] =
{
  "Very Good",
  "Good",
  "Fair",
  "Poor",
  "Very Poor",
  "Hazardous",
};
const char *CANADA_AQHI_TXT[4] =
{
  "Low",
  "Moderate",
  "High",
  "Very High",
};
const char *EUROPEAN_UNION_CAQI_TXT[5] =
{
  "Very Low",
  "Low",
  "Medium",
  "High",
  "Very High",
};
const char *HONG_KONG_AQHI_TXT[5] =
{
  "Low",
  "Moderate",
  "High",
  "Very High",
  "Hazardous",
};
const char *INDIA_AQI_TXT[6] =
{
  "Good",
  "Satisfactory",
  "Moderate",
  "Poor",
  "Very Poor",
  "Severe",
};
const char *CHINA_AQI_TXT[6] =
{
  "Excellent",
  "Good",
  "Lightly Polluted",
  "Moderately Polluted",
  "Heavily Polluted",
  "Severely Polluted",
};
const char *SINGAPORE_PSI_TXT[5] =
{
  "Good",
  "Moderate",
  "Unhealthy",
  "Very Unhealthy",
  "Hazardous",
};
const char *SOUTH_KOREA_CAI_TXT[4] =
{
  "Good",
  "Medium",
  "Unhealthy",
  "Very Unhealthy",
};
const char *UNITED_KINGDOM_DAQI_TXT[4] =
{
  "Low",
  "Moderate",
  "High",
  "Very High",
};
const char *UNITED_STATES_AQI_TXT[6] =
{
  "Good",
  "Moderate",
  "Unhealthy for Sensitive Groups",
  "Unhealthy",
  "Very Unhealthy",
  "Hazardous",
};
} // end extern "C"

// COMPASS POINT
const char *COMPASS_POINT_NOTATION[32] = {
//   0°   11.25°   22.5°   33.75°   45°   56.25°   67.5°   78.75°
   "N",  "NbE",  "NNE",   "NEbN",  "NE", "NEbE",  "ENE",  "EbN",
//  90°  101.25°  112.5°  123.75°  135°  146.25°  157.5°  168.75°
   "E",  "EbS",   "ESE",  "SEbE",  "SE", "SEbS",  "SSE",  "SbE",
// 180°  191.25°  202.5°  213.75°  225°  236.25°  247.5°  258.75°
   "S",  "SbW",   "SSW",  "SWbS",  "SW", "SWbW",  "WSW",  "WbS",
// 270°  281.25°  292.5°  303.75°  315°  326.25°  337.5°  348.75°
   "W",  "WbN",   "WNW",  "NWbW",  "NW", "NWbN",  "NNW",  "NbW",
};

// HTTP CLIENT ERRORS
const char *TXT_HTTPC_ERROR_CONNECTION_REFUSED   = "连接被拒绝";
const char *TXT_HTTPC_ERROR_SEND_HEADER_FAILED   = "发送头部失败";
const char *TXT_HTTPC_ERROR_SEND_PAYLOAD_FAILED  = "发送内容失败";
const char *TXT_HTTPC_ERROR_NOT_CONNECTED        = "未连接";
const char *TXT_HTTPC_ERROR_CONNECTION_LOST      = "连接丢失";
const char *TXT_HTTPC_ERROR_NO_STREAM            = "无数据流";
const char *TXT_HTTPC_ERROR_NO_HTTP_SERVER       = "无 HTTP 服务器";
const char *TXT_HTTPC_ERROR_TOO_LESS_RAM         = "RAM 不足";
const char *TXT_HTTPC_ERROR_ENCODING             = "不支持的传输编码";
const char *TXT_HTTPC_ERROR_STREAM_WRITE         = "写入数据流失败";
const char *TXT_HTTPC_ERROR_READ_TIMEOUT         = "读取超时";

// HTTP RESPONSE STATUS CODES
// 1xx - Informational Responses
const char *TXT_HTTP_RESPONSE_100 = "继续";
const char *TXT_HTTP_RESPONSE_101 = "切换协议";
const char *TXT_HTTP_RESPONSE_102 = "处理中";
const char *TXT_HTTP_RESPONSE_103 = "早期提示";
// 2xx - Successful Responses
const char *TXT_HTTP_RESPONSE_200 = "成功";
const char *TXT_HTTP_RESPONSE_201 = "已创建";
const char *TXT_HTTP_RESPONSE_202 = "已接受";
const char *TXT_HTTP_RESPONSE_203 = "非授权信息";
const char *TXT_HTTP_RESPONSE_204 = "无内容";
const char *TXT_HTTP_RESPONSE_205 = "重置内容";
const char *TXT_HTTP_RESPONSE_206 = "部分内容";
const char *TXT_HTTP_RESPONSE_207 = "多状态";
const char *TXT_HTTP_RESPONSE_208 = "已报告";
const char *TXT_HTTP_RESPONSE_226 = "IM 已使用";
// 3xx - Redirection Responses
const char *TXT_HTTP_RESPONSE_300 = "多种选择";
const char *TXT_HTTP_RESPONSE_301 = "永久移动";
const char *TXT_HTTP_RESPONSE_302 = "临时重定向";
const char *TXT_HTTP_RESPONSE_303 = "查看其它位置";
const char *TXT_HTTP_RESPONSE_304 = "未修改";
const char *TXT_HTTP_RESPONSE_305 = "使用代理";
const char *TXT_HTTP_RESPONSE_307 = "临时重定向";
const char *TXT_HTTP_RESPONSE_308 = "永久重定向";
// 4xx - Client Error Responses
const char *TXT_HTTP_RESPONSE_400 = "错误请求";
const char *TXT_HTTP_RESPONSE_401 = "未授权";
const char *TXT_HTTP_RESPONSE_402 = "需要付款";
const char *TXT_HTTP_RESPONSE_403 = "拒绝访问";
const char *TXT_HTTP_RESPONSE_404 = "未找到";
const char *TXT_HTTP_RESPONSE_405 = "方法不允许";
const char *TXT_HTTP_RESPONSE_406 = "不可接受";
const char *TXT_HTTP_RESPONSE_407 = "需要代理授权";
const char *TXT_HTTP_RESPONSE_408 = "请求超时";
const char *TXT_HTTP_RESPONSE_409 = "冲突";
const char *TXT_HTTP_RESPONSE_410 = "已删除";
const char *TXT_HTTP_RESPONSE_411 = "需要长度";
const char *TXT_HTTP_RESPONSE_412 = "前提条件失败";
const char *TXT_HTTP_RESPONSE_413 = "内容过大";
const char *TXT_HTTP_RESPONSE_414 = "URI 过长";
const char *TXT_HTTP_RESPONSE_415 = "不支持的媒体类型";
const char *TXT_HTTP_RESPONSE_416 = "范围不满足";
const char *TXT_HTTP_RESPONSE_417 = "期望失败";
const char *TXT_HTTP_RESPONSE_418 = "我是茶壶";
const char *TXT_HTTP_RESPONSE_421 = "错误的请求方向";
const char *TXT_HTTP_RESPONSE_422 = "不可处理的内容";
const char *TXT_HTTP_RESPONSE_423 = "资源被锁";
const char *TXT_HTTP_RESPONSE_424 = "依赖失败";
const char *TXT_HTTP_RESPONSE_425 = "过早";
const char *TXT_HTTP_RESPONSE_426 = "需要升级";
const char *TXT_HTTP_RESPONSE_428 = "需要前提条件";
const char *TXT_HTTP_RESPONSE_429 = "请求过多";
const char *TXT_HTTP_RESPONSE_431 = "请求头过大";
const char *TXT_HTTP_RESPONSE_451 = "因法律原因不可用";
// 5xx - Server Error Responses
const char *TXT_HTTP_RESPONSE_500 = "内部服务器错误";
const char *TXT_HTTP_RESPONSE_501 = "未实现";
const char *TXT_HTTP_RESPONSE_502 = "错误网关";
const char *TXT_HTTP_RESPONSE_503 = "服务不可用";
const char *TXT_HTTP_RESPONSE_504 = "网关超时";
const char *TXT_HTTP_RESPONSE_505 = "HTTP 版本不支持";
const char *TXT_HTTP_RESPONSE_506 = "变体也在协商";
const char *TXT_HTTP_RESPONSE_507 = "存储不足";
const char *TXT_HTTP_RESPONSE_508 = "检测到循环";
const char *TXT_HTTP_RESPONSE_510 = "未扩展";
const char *TXT_HTTP_RESPONSE_511 = "需要网络认证";

// ARDUINOJSON DESERIALIZATION ERROR CODES
const char *TXT_DESERIALIZATION_ERROR_OK               = "反序列化成功";
const char *TXT_DESERIALIZATION_ERROR_EMPTY_INPUT      = "输入为空";
const char *TXT_DESERIALIZATION_ERROR_INCOMPLETE_INPUT = "输入不完整";
const char *TXT_DESERIALIZATION_ERROR_INVALID_INPUT    = "输入无效";
const char *TXT_DESERIALIZATION_ERROR_NO_MEMORY        = "内存不足";
const char *TXT_DESERIALIZATION_ERROR_TOO_DEEP         = "层级过深";

// WIFI STATUS
const char *TXT_WL_NO_SHIELD       = "没有网卡";
const char *TXT_WL_IDLE_STATUS     = "空闲";
const char *TXT_WL_NO_SSID_AVAIL   = "无可用 SSID";
const char *TXT_WL_SCAN_COMPLETED  = "扫描完成";
const char *TXT_WL_CONNECTED       = "已连接";
const char *TXT_WL_CONNECT_FAILED  = "连接失败";
const char *TXT_WL_CONNECTION_LOST = "连接丢失";
const char *TXT_WL_DISCONNECTED    = "已断开";
