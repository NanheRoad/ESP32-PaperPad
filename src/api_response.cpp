/* 中国气象台 API 解析 */
#include <ArduinoJson.h>
#include "api_response.h"
#include "config.h"

// 若 API 未提供风向角度，则根据中文风向转换
static int cnWindToDeg(const String &s)
{
  if (s.indexOf("北") != -1 && s.indexOf("东") == -1 && s.indexOf("西") == -1) return 0;
  if (s.indexOf("东北") != -1) return 45;
  if (s.indexOf("东") != -1 && s.indexOf("南") == -1) return 90;
  if (s.indexOf("东南") != -1) return 135;
  if (s.indexOf("南") != -1 && s.indexOf("东") == -1 && s.indexOf("西") == -1) return 180;
  if (s.indexOf("西南") != -1) return 225;
  if (s.indexOf("西") != -1 && s.indexOf("北") == -1) return 270;
  if (s.indexOf("西北") != -1) return 315;
  return 0;
}

static void fillCMAWeatherFromRoot(JsonObject root, cma_weather_t &r)
{
  JsonObject nowinfo = root["nowinfo"];
  r.code = root["code"] | 0;
  r.message = root["msg"] | String();
  r.precipitation = nowinfo["precipitation"] | root["precipitation"] | 0.0f;
  r.temperature = nowinfo["temperature"] | root["temperature"] | 0.0f;
  r.humidity = nowinfo["humidity"] | root["humidity"] | 0;
  r.windDirection = nowinfo["windDirection"] | root["windDirection"] | String();
  r.windDirectionDegree = nowinfo["windDirectionDegree"]
                           | root["windDirectionDegree"]
                           | cnWindToDeg(r.windDirection);
  r.windSpeed = nowinfo["windSpeed"] | root["windSpeed"] | 0.0f;
  r.windScale = nowinfo["windScale"] | root["windScale"] | String();
  r.place = root["place"] | root["name"] | root["shi"] | String();
  r.weather1 = root["weather1"] | String();
  r.weather2 = root["weather2"] | String();
}

/* 反序列化中国气象台天气数据 */
DeserializationError deserializeCMAWeather(WiFiClient &json,
                                           cma_weather_t &r)
{
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);
#if DEBUG_LEVEL >= 2
  serializeJsonPretty(doc, Serial);
#endif
  if (error) {
    return error;
  }

  JsonObject root = doc.as<JsonObject>();
  fillCMAWeatherFromRoot(root, r);
  return error;
} // end deserializeCMAWeather

/* 反序列化中国气象台天气数据（字符串版本） */
DeserializationError deserializeCMAWeather(const String &json,
                                           cma_weather_t &r)
{
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);
#if DEBUG_LEVEL >= 2
  serializeJsonPretty(doc, Serial);
#endif
  if (error) {
    return error;
  }

  JsonObject root = doc.as<JsonObject>();
  fillCMAWeatherFromRoot(root, r);
  return error;
} // end deserializeCMAWeather
