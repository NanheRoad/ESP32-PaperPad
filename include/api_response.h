/* 中国气象台 API 响应结构声明 */
#ifndef __API_RESPONSE_H__
#define __API_RESPONSE_H__

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>

// 中国气象台天气数据结构
typedef struct {
  float  precipitation;      // 降水量 mm
  float  temperature;        // 温度 °C
  float  pressure;           // 气压 hPa
  int    humidity;           // 湿度 %
  String windDirection;      // 风向文字
  int    windDirectionDegree;// 风向角度
  float  windSpeed;          // 风速 m/s
  String windScale;          // 风力级别描述
  int    code;               // 状态码
  String message;            // 消息内容
  String place;              // 地区
  String weather1;           // 当日天气1
  String weather2;           // 当日天气2
} cma_weather_t;

DeserializationError deserializeCMAWeather(WiFiClient &json,
                                           cma_weather_t &r);

#endif
