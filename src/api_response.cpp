/* API response deserialization for esp32-weather-epd.
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

#include <vector>
#include <ArduinoJson.h>
#include "api_response.h"
#include "config.h"

// 将中文风向转换为角度
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

// 简单映射中文天气到 OWM ID
static int cnWeatherToId(const String &s)
{
  if (s.indexOf("晴") != -1) return 800;
  if (s.indexOf("多云") != -1) return 801;
  if (s.indexOf("阴") != -1) return 804;
  if (s.indexOf("雷") != -1) return 210;
  if (s.indexOf("雪") != -1) return 600;
  if (s.indexOf("雨") != -1) return 500;
  return 800;
}

static String iconFromId(int id)
{
  if (id >= 200 && id < 300) return "11d";
  if (id >= 300 && id < 600) return "09d";
  if (id >= 600 && id < 700) return "13d";
  if (id >= 700 && id < 800) return "50d";
  if (id == 800) return "01d";
  if (id == 801) return "02d";
  if (id == 802) return "03d";
  return "04d";
}

DeserializationError deserializeCMAWeather(WiFiClient &json,
                                           owm_resp_onecall_t &r)
{
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);
#if DEBUG_LEVEL >= 2
  serializeJsonPretty(doc, Serial);
#endif
  if (error) {
    return error;
  }

  JsonObject root = doc;

  // 当前天气
  r.current.temp       = root["tem"].as<float>();
  r.current.humidity   = root["hum"].as<int>();
  r.current.wind_speed = String(root["win_speed"].as<const char *>()).toFloat();
  r.current.wind_deg   = cnWindToDeg(root["win"].as<const char *>());
  r.current.weather.description = root["wea"].as<const char *>();
  r.current.weather.id  = cnWeatherToId(root["wea"].as<const char *>());
  r.current.weather.icon = iconFromId(r.current.weather.id);

  // 日预报
  int i = 0;
  for (JsonObject day : root["data"].as<JsonArray>())
  {
    r.daily[i].temp.max = day["tem_day"].as<float>();
    r.daily[i].temp.min = day["tem_night"].as<float>();
    r.daily[i].weather.description = day["wea"].as<const char *>();
    r.daily[i].weather.id = cnWeatherToId(day["wea"].as<const char *>());
    r.daily[i].weather.icon = iconFromId(r.daily[i].weather.id);
    r.daily[i].wind_speed = String(day["win_speed"].as<const char *>()).toFloat();
    r.daily[i].wind_deg = cnWindToDeg(day["win"].as<const char *>());
    if (++i == OWM_NUM_DAILY) break;
  }

  return error;
} // end deserializeCMAWeather

DeserializationError deserializeAirQuality(WiFiClient &json,
                                           owm_resp_air_pollution_t &r)
{
  int i = 0;

  JsonDocument doc;

  DeserializationError error = deserializeJson(doc, json);
#if DEBUG_LEVEL >= 1
  Serial.println("[debug] doc.overflowed() : "
                 + String(doc.overflowed()));
#endif
#if DEBUG_LEVEL >= 2
  serializeJsonPretty(doc, Serial);
#endif
  if (error) {
    return error;
  }

  r.coord.lat = doc["coord"]["lat"].as<float>();
  r.coord.lon = doc["coord"]["lon"].as<float>();

  for (JsonObject list : doc["list"].as<JsonArray>())
  {

    r.main_aqi[i] = list["main"]["aqi"].as<int>();

    JsonObject list_components = list["components"];
    r.components.co[i]    = list_components["co"].as<float>();
    r.components.no[i]    = list_components["no"].as<float>();
    r.components.no2[i]   = list_components["no2"].as<float>();
    r.components.o3[i]    = list_components["o3"].as<float>();
    r.components.so2[i]   = list_components["so2"].as<float>();
    r.components.pm2_5[i] = list_components["pm2_5"].as<float>();
    r.components.pm10[i]  = list_components["pm10"].as<float>();
    r.components.nh3[i]   = list_components["nh3"].as<float>();

    r.dt[i] = list["dt"].as<int64_t>();

    if (i == OWM_NUM_AIR_POLLUTION - 1)
    {
      break;
    }
    ++i;
  }

  return error;
} // end deserializeAirQuality

