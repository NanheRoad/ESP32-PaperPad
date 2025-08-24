/* esp32-weather-epd API 响应解析声明。
 * Copyright (C) 2022-2023  Luke Marzen
 *
 * 本程序为自由软件：你可以根据 GNU GPL v3.0 或其任一后续版本
 * 重新分发和/或修改本程序。
 *
 * 本程序按“原样”提供，不附带任何明示或暗示的担保。
 *
 * 你应已收到 GNU 通用公共许可证的副本，
 * 若没有，请查看 <https://www.gnu.org/licenses/>。
 */

#ifndef __API_RESPONSE_H__
#define __API_RESPONSE_H__

#include <cstdint>
#include <vector>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

#define OWM_NUM_MINUTELY       1  // 最多 1 条逐分钟数据（保留）
#define OWM_NUM_HOURLY        48  // 最多 48 条逐小时数据
#define OWM_NUM_DAILY          8  // 最多 8 天预报
#define OWM_NUM_ALERTS         8  // 最多 8 条天气警报
#define OWM_NUM_AIR_POLLUTION 24  // 空气质量小时数据数量

typedef struct owm_weather
{
  int     id;               // 天气现象 ID
  String  main;             // 天气大类（雨、雪等）
  String  description;      // 具体描述
  String  icon;             // 图标 ID
} owm_weather_t;

/*
 * 温度单位：默认开尔文；公制摄氏；英制华氏。
 */
typedef struct owm_temp
{
  float   morn;             // 早晨温度
  float   day;              // 白天温度
  float   eve;              // 傍晚温度
  float   night;            // 夜间温度
  float   min;              // 日最低温
  float   max;              // 日最高温
} owm_temp_t;

/*
 * 体感温度，反映人体对天气的感知。单位同上。
 */
typedef struct owm_feels_like
{
  float   morn;             // 早晨体感温度
  float   day;              // 白天体感温度
  float   eve;              // 傍晚体感温度
  float   night;            // 夜间体感温度
} owm_owm_feels_like_t;

/* 当前天气数据 */
typedef struct owm_current
{
  int64_t dt;               // 数据时间（Unix, UTC）
  int64_t sunrise;          // 日出时间
  int64_t sunset;           // 日落时间
  float   temp;             // 温度
  float   feels_like;       // 体感温度
  int     humidity;         // 相对湿度 %
  float   dew_point;        // 露点温度
  int     clouds;           // 云量 %
  float   uvi;              // UV 指数
  int     visibility;       // 能见度 米
  float   wind_speed;       // 风速
  float   wind_gust;        // 阵风
  int     wind_deg;         // 风向 度
  float   rain_1h;          // 最近1小时降雨量 mm
  float   snow_1h;          // 最近1小时降雪量 mm
  owm_weather_t         weather; // 天气描述
} owm_current_t;

/* 逐分钟预报数据 */
typedef struct owm_minutely
{
  int64_t dt;               // 时间
  float   precipitation;    // 降水量 mm
} owm_minutely_t;

/* 逐小时预报数据 */
typedef struct owm_hourly
{
  int64_t dt;               // 时间
  float   temp;             // 温度
  float   feels_like;       // 体感温度
  int     humidity;         // 相对湿度 %
  float   dew_point;        // 露点
  int     clouds;           // 云量 %
  float   uvi;              // UV 指数
  int     visibility;       // 能见度 米
  float   wind_speed;       // 风速
  float   wind_gust;        // 阵风
  int     wind_deg;         // 风向 度
  float   pop;              // 降水概率 [0,1]
  float   rain_1h;          // 最近1小时降雨量 mm
  float   snow_1h;          // 最近1小时降雪量 mm
  owm_weather_t         weather; // 天气描述
} owm_hourly_t;

/* 每日预报数据 */
typedef struct owm_daily
{
  int64_t dt;               // 时间
  int64_t sunrise;          // 日出时间
  int64_t sunset;           // 日落时间
  int64_t moonrise;         // 月升时间
  int64_t moonset;          // 月落时间
  float   moon_phase;       // 月相
  owm_temp_t            temp;       // 温度信息
  owm_owm_feels_like_t  feels_like; // 体感温度
  int     humidity;         // 相对湿度 %
  float   dew_point;        // 露点
  int     clouds;           // 云量 %
  float   uvi;              // UV 指数
  int     visibility;       // 能见度 米
  float   wind_speed;       // 风速
  float   wind_gust;        // 阵风
  int     wind_deg;         // 风向 度
  float   pop;              // 降水概率 [0,1]
  float   rain;             // 降雨量 mm
  float   snow;             // 降雪量 mm
  owm_weather_t         weather;   // 天气描述
} owm_daily_t;

/* 国家级天气警报数据 */
typedef struct owm_alerts
{
  String  sender_name;      // 发布机构
  String  event;            // 事件名称
  int64_t start;            // 警报开始时间
  int64_t end;              // 警报结束时间
  String  description;      // 详细描述
  String  tags;             // 天气类型
} owm_alerts_t;

/* 综合天气响应结构 */
typedef struct owm_resp_onecall
{
  float   lat;              // 纬度
  float   lon;              // 经度
  String  timezone;         // 时区名称
  int     timezone_offset;  // 与 UTC 偏移秒数
  owm_current_t   current;  // 当前天气
  // owm_minutely_t  minutely[OWM_NUM_MINUTELY]; // 逐分钟数据

  owm_hourly_t    hourly[OWM_NUM_HOURLY]; // 小时预报
  owm_daily_t     daily[OWM_NUM_DAILY];   // 日预报
  std::vector<owm_alerts_t> alerts;       // 警报列表
} owm_resp_onecall_t;

/* 地理坐标 */
typedef struct owm_coord
{
  float   lat;
  float   lon;
} owm_coord_t;

typedef struct owm_components
{
  float   co[OWM_NUM_AIR_POLLUTION];    // 一氧化碳 CO μg/m^3
  float   no[OWM_NUM_AIR_POLLUTION];    // 一氧化氮 NO μg/m^3
  float   no2[OWM_NUM_AIR_POLLUTION];   // 二氧化氮 NO2 μg/m^3
  float   o3[OWM_NUM_AIR_POLLUTION];    // 臭氧 O3 μg/m^3
  float   so2[OWM_NUM_AIR_POLLUTION];   // 二氧化硫 SO2 μg/m^3
  float   pm2_5[OWM_NUM_AIR_POLLUTION]; // PM2.5 μg/m^3
  float   pm10[OWM_NUM_AIR_POLLUTION];  // PM10 μg/m^3
  float   nh3[OWM_NUM_AIR_POLLUTION];   // 氨气 NH3 μg/m^3
} owm_components_t;

/* 空气质量响应结构 */
typedef struct owm_resp_air_pollution
{
  owm_coord_t      coord;                             // 坐标
  int              main_aqi[OWM_NUM_AIR_POLLUTION];   // AQI 等级 1-5
  owm_components_t components;                        // 污染物浓度
  int64_t          dt[OWM_NUM_AIR_POLLUTION];         // 时间戳
} owm_resp_air_pollution_t;

DeserializationError deserializeCMAWeather(WiFiClient &json,
                                           owm_resp_onecall_t &r);
DeserializationError deserializeAirQuality(WiFiClient &json,
                                           owm_resp_air_pollution_t &r);


#endif

