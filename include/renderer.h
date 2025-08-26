/* 墨水屏渲染函数声明 */
#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <vector>
#include <Arduino.h>
#include <time.h>
#include "api_response.h"
#include "config.h"

#ifdef DISP_BW_V2
  #define DISP_WIDTH  800
  #define DISP_HEIGHT 480
  #include <GxEPD2_BW.h>
  extern GxEPD2_BW<GxEPD2_750_T7,
                   GxEPD2_750_T7::HEIGHT> display;
#endif
#ifdef DISP_3C_B
  #define DISP_WIDTH  800
  #define DISP_HEIGHT 480
  #include <GxEPD2_3C.h>
  extern GxEPD2_3C<GxEPD2_750c_Z08,
                   GxEPD2_750c_Z08::HEIGHT / 2> display;
#endif
#ifdef DISP_7C_F
  #define DISP_WIDTH  800
  #define DISP_HEIGHT 480
  #include <GxEPD2_7C.h>
  extern GxEPD2_7C<GxEPD2_730c_GDEY073D46,
                   GxEPD2_730c_GDEY073D46::HEIGHT / 4> display;
#endif
#ifdef DISP_BW_V1
  #define DISP_WIDTH  640
  #define DISP_HEIGHT 384
  #include <GxEPD2_BW.h>
  extern GxEPD2_BW<GxEPD2_750,
                   GxEPD2_750::HEIGHT> display;
#endif

#ifndef ACCENT_COLOR
  #define ACCENT_COLOR GxEPD_BLACK
#endif

typedef enum alignment { LEFT, RIGHT, CENTER } alignment_t;

uint16_t getStringWidth(const String &text);
uint16_t getStringHeight(const String &text);
void drawString(int16_t x, int16_t y, const String &text, alignment_t alignment,
                uint16_t color=GxEPD_BLACK);
void drawMultiLnString(int16_t x, int16_t y, const String &text,
                       alignment_t alignment, uint16_t max_width,
                       uint16_t max_lines, int16_t line_spacing,
                       uint16_t color=GxEPD_BLACK);
void initDisplay();
void powerOffDisplay();
void drawCurrentWeather(const cma_weather_t &weather,
                        float inTemp, float inHumidity);
void drawLocationDate(const String &city, const String &date);
void drawStatusBar(const String &statusStr, const String &refreshTimeStr,
                   int rssi, uint32_t batVoltage);
void drawError(const uint8_t *bitmap_196x196,
               const String &errMsgLn1, const String &errMsgLn2="");

#endif
