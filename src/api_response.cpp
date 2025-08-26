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
  r.code = root["code"] | 0;
  r.message = root["msg"] | String();
  r.precipitation = root["precipitation"] | 0.0f;
  r.temperature = root["temperature"] | 0.0f;
  r.humidity = root["humidity"] | 0;
  r.windDirection = root["windDirection"] | String();
  r.windDirectionDegree = root["windDirectionDegree"]
                           | cnWindToDeg(r.windDirection);
  r.windSpeed = root["windSpeed"] | 0.0f;
  r.windScale = root["windScale"] | String();
  r.place = root["place"] | String();
  r.weather1 = root["weather1"] | String();
  r.weather2 = root["weather2"] | String();
  return error;
} // end deserializeCMAWeather

