/* 简化渲染器：根据中国气象台数据在墨水屏上绘制内容 */
#include "_locale.h"
#include "_strftime.h"
#include "renderer.h"
#include "conversions.h"
#include "display_utils.h"
#include "config.h"
#include <U8g2_for_Adafruit_GFX.h>

// 字体
#include FONT_HEADER
// 图标
#include "icons/icons_16x16.h"
#include "icons/icons_24x24.h"
#include "icons/icons_32x32.h"
#include "icons/icons_48x48.h"
#include "icons/icons_196x196.h"

#ifdef DISP_BW_V2
  GxEPD2_BW<GxEPD2_750_T7,
            GxEPD2_750_T7::HEIGHT> display(
    GxEPD2_750_T7(PIN_EPD_CS,
                  PIN_EPD_DC,
                  PIN_EPD_RST,
                  PIN_EPD_BUSY));
#endif
#ifdef DISP_3C_B
  GxEPD2_3C<GxEPD2_750c_Z08,
            GxEPD2_750c_Z08::HEIGHT / 2> display(
    GxEPD2_750c_Z08(PIN_EPD_CS,
                    PIN_EPD_DC,
                    PIN_EPD_RST,
                    PIN_EPD_BUSY));
#endif
#ifdef DISP_7C_F
  GxEPD2_7C<GxEPD2_730c_GDEY073D46,
            GxEPD2_730c_GDEY073D46::HEIGHT / 4> display(
    GxEPD2_730c_GDEY073D46(PIN_EPD_CS,
                           PIN_EPD_DC,
                           PIN_EPD_RST,
                           PIN_EPD_BUSY));
#endif
#ifdef DISP_BW_V1
  GxEPD2_BW<GxEPD2_750,
            GxEPD2_750::HEIGHT> display(
    GxEPD2_750(PIN_EPD_CS,
               PIN_EPD_DC,
               PIN_EPD_RST,
               PIN_EPD_BUSY));
#endif

static U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;
static bool u8g2FontsReady = false;

static bool hasNonAscii(const String &text)
{
  for (size_t i = 0; i < text.length(); ++i)
  {
    if (static_cast<uint8_t>(text[i]) & 0x80) return true;
  }
  return false;
}

static void ensureUtf8Font(uint16_t color)
{
  if (!u8g2FontsReady)
  {
    u8g2Fonts.begin(display);
    u8g2Fonts.setFontMode(1); // transparent background
    u8g2FontsReady = true;
  }
  // WenQuanYi 位图中文字体，支持 GB2312 范围汉字与常见符号。
  u8g2Fonts.setFont(u8g2_font_wqy12_t_gb2312);
  u8g2Fonts.setForegroundColor(color);
  u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
}

static float convertTempC(float t)
{
#if defined(UNITS_TEMP_KELVIN)
  return celsius_to_kelvin(t);
#elif defined(UNITS_TEMP_FAHRENHEIT)
  return celsius_to_fahrenheit(t);
#else
  return t;
#endif
}

static const char *getTempUnit()
{
#if defined(UNITS_TEMP_KELVIN)
  return TXT_UNITS_TEMP_KELVIN;
#elif defined(UNITS_TEMP_FAHRENHEIT)
  return TXT_UNITS_TEMP_FAHRENHEIT;
#else
  return TXT_UNITS_TEMP_CELSIUS;
#endif
}

static float convertWindSpeedMs(float s)
{
#if defined(UNITS_SPEED_FEETPERSECOND)
  return meterspersecond_to_feetpersecond(s);
#elif defined(UNITS_SPEED_KILOMETERSPERHOUR)
  return meterspersecond_to_kilometersperhour(s);
#elif defined(UNITS_SPEED_MILESPERHOUR)
  return meterspersecond_to_milesperhour(s);
#elif defined(UNITS_SPEED_KNOTS)
  return meterspersecond_to_knots(s);
#elif defined(UNITS_SPEED_BEAUFORT)
  return static_cast<float>(meterspersecond_to_beaufort(s));
#else
  return s;
#endif
}

static const char *getWindSpeedUnit()
{
#if defined(UNITS_SPEED_FEETPERSECOND)
  return TXT_UNITS_SPEED_FEETPERSECOND;
#elif defined(UNITS_SPEED_KILOMETERSPERHOUR)
  return TXT_UNITS_SPEED_KILOMETERSPERHOUR;
#elif defined(UNITS_SPEED_MILESPERHOUR)
  return TXT_UNITS_SPEED_MILESPERHOUR;
#elif defined(UNITS_SPEED_KNOTS)
  return TXT_UNITS_SPEED_KNOTS;
#elif defined(UNITS_SPEED_BEAUFORT)
  return TXT_UNITS_SPEED_BEAUFORT;
#else
  return TXT_UNITS_SPEED_METERSPERSECOND;
#endif
}

static float convertPrecipMm(float mm)
{
#if defined(UNITS_HOURLY_PRECIP_CENTIMETERS)
  return millimeters_to_centimeters(mm);
#elif defined(UNITS_HOURLY_PRECIP_INCHES)
  return millimeters_to_inches(mm);
#else
  return mm;
#endif
}

static const char *getPrecipUnit()
{
#if defined(UNITS_HOURLY_PRECIP_CENTIMETERS)
  return TXT_UNITS_PRECIP_CENTIMETERS;
#elif defined(UNITS_HOURLY_PRECIP_INCHES)
  return TXT_UNITS_PRECIP_INCHES;
#else
  return TXT_UNITS_PRECIP_MILLIMETERS;
#endif
}

/* 计算字符串宽度 */
uint16_t getStringWidth(const String &text)
{
  if (hasNonAscii(text))
  {
    ensureUtf8Font(GxEPD_BLACK);
    return static_cast<uint16_t>(u8g2Fonts.getUTF8Width(text.c_str()));
  }
  int16_t x1, y1; uint16_t w, h;
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  return w;
}

/* 计算字符串高度 */
uint16_t getStringHeight(const String &text)
{
  if (hasNonAscii(text))
  {
    ensureUtf8Font(GxEPD_BLACK);
    const int16_t h = u8g2Fonts.getFontAscent() - u8g2Fonts.getFontDescent();
    return static_cast<uint16_t>(h > 0 ? h : 16);
  }
  int16_t x1, y1; uint16_t w, h;
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  return h;
}

/* 按对齐方式绘制字符串 */
void drawString(int16_t x, int16_t y, const String &text, alignment_t align,
                uint16_t color)
{
  if (hasNonAscii(text))
  {
    ensureUtf8Font(color);
    const int16_t w = u8g2Fonts.getUTF8Width(text.c_str());
    if (align == RIGHT) x -= w;
    if (align == CENTER) x -= w / 2;
    u8g2Fonts.setCursor(x, y);
    u8g2Fonts.print(text);
    return;
  }
  int16_t x1, y1; uint16_t w, h;
  display.setTextColor(color);
  display.getTextBounds(text, x, y, &x1, &y1, &w, &h);
  if (align == RIGHT) x -= w;
  if (align == CENTER) x -= w / 2;
  display.setCursor(x, y);
  display.print(text);
}

/* 多行文本绘制 */
void drawMultiLnString(int16_t x, int16_t y, const String &text,
                       alignment_t align, uint16_t max_w,
                       uint16_t max_lines, int16_t line_spacing,
                       uint16_t color)
{
  uint16_t current = 0; String remain = text;
  while (current < max_lines && !remain.isEmpty())
  {
    uint16_t w = getStringWidth(remain);
    int endIndex = remain.length();
    String sub = remain; int splitAt = 0; int keep = 0;
    while (w > max_w && splitAt != -1)
    {
      if (keep) { sub.remove(sub.length() - 1); }
      splitAt = current < max_lines - 1 ?
                std::max(sub.lastIndexOf(" "), sub.lastIndexOf("-")) :
                sub.lastIndexOf(" ");
      if (splitAt != -1)
      {
        endIndex = splitAt;
        sub = sub.substring(0, endIndex + 1);
        char last = sub.charAt(endIndex);
        if (last == ' ') { keep = 0; sub.remove(endIndex); --endIndex; }
        else if (last == '-') { keep = 1; }
        if (current == max_lines - 1)
        {
          w = getStringWidth(sub + "...");
          if (w <= max_w) sub += "...";
        }
        else
        {
          w = getStringWidth(sub);
        }
      }
    }
    drawString(x, y + current * line_spacing, sub, align, color);
    remain = remain.substring(endIndex + 2 - keep);
    ++current;
  }
}

/* 初始化墨水屏 */
void initDisplay()
{
  if (PIN_EPD_PWR >= 0)
  {
    pinMode(PIN_EPD_PWR, OUTPUT);
    digitalWrite(PIN_EPD_PWR, HIGH);
  }
  // 先重映射 SPI 引脚，再初始化 GxEPD2。
  // 否则 ESP32 默认 MISO=GPIO19 会与 EPD_DC(GPIO19) 冲突，导致屏不刷新。
  SPI.end();
  SPI.begin(PIN_EPD_SCK, PIN_EPD_MISO, PIN_EPD_MOSI, PIN_EPD_CS);
  // 固定使用较稳妥的 4MHz SPI 时钟，便于长线和面包板场景稳定刷新。
  display.epd2.selectSPI(SPI, SPISettings(4000000, MSBFIRST, SPI_MODE0));
#ifdef DRIVER_WAVESHARE
  // Waveshare "clever reset" 参考参数：2ms 复位脉冲
  display.init(0, true, 2, false);
#endif
#ifdef DRIVER_DESPI_C02
  display.init(0, true, 10, false);
#endif
  display.setRotation(0);
  display.setTextSize(1);
  display.setTextColor(GxEPD_BLACK);
  display.setTextWrap(false);
  u8g2FontsReady = false;
  ensureUtf8Font(GxEPD_BLACK);
  display.setFullWindow();
  display.firstPage();
}

/* 关闭墨水屏电源 */
void powerOffDisplay()
{
  display.hibernate();
  if (PIN_EPD_PWR >= 0)
  {
    digitalWrite(PIN_EPD_PWR, LOW);
  }
}

/* 绘制当前天气和室内传感器数据 */
void drawCurrentWeather(const cma_weather_t &w,
                        float inTemp, float inHumidity)
{
  const float outTemp = convertTempC(w.temperature);
  const float indoorTemp = convertTempC(inTemp);
  const float windSpeed = convertWindSpeedMs(w.windSpeed);
  const float precip = convertPrecipMm(w.precipitation);

  display.setFont(&FONT_26pt8b);
  drawString(10, 40, w.weather1 + "/" + w.weather2, LEFT);
  display.setFont(&FONT_16pt8b);
  drawString(10, 80, String("温度 ") + String(outTemp, 1) + getTempUnit()
                      + "  湿度 " + String(w.humidity) + "%", LEFT);
  drawString(10, 110, String("风 ") + w.windDirection + " " +
                      String(windSpeed, 1) + getWindSpeedUnit(), LEFT);
  drawString(10, 140, String("降水 ") + String(precip, 1) + getPrecipUnit(), LEFT);
  drawString(10, 170, String("室内温度 ") + String(indoorTemp, 1) + getTempUnit()
                      + "  室内湿度 " + String(inHumidity,1) + "%", LEFT);
}

/* 绘制城市与日期 */
void drawLocationDate(const String &city, const String &date)
{
  display.setFont(&FONT_16pt8b);
  drawString(DISP_WIDTH - 2, 23, city, RIGHT, ACCENT_COLOR);
  display.setFont(&FONT_12pt8b);
  drawString(DISP_WIDTH - 2, 30 + 4 + 17, date, RIGHT);
}

/* 绘制状态栏：刷新时间、WiFi、电池等 */
void drawStatusBar(const String &statusStr, const String &refreshTimeStr,
                   int rssi, uint32_t batVoltage)
{
  String dataStr; uint16_t dataColor = GxEPD_BLACK;
  display.setFont(&FONT_6pt8b);
  int pos = DISP_WIDTH - 2; const int sp = 2;
#if BATTERY_MONITORING
  uint32_t batPercent = calcBatPercent(batVoltage,
                                       MIN_BATTERY_VOLTAGE,
                                       MAX_BATTERY_VOLTAGE);
  dataStr = String(batPercent) + "%";
  drawString(pos, DISP_HEIGHT - 1 - 2, dataStr, RIGHT, dataColor);
  pos -= getStringWidth(dataStr) + 25;
  display.drawInvertedBitmap(pos, DISP_HEIGHT - 1 - 17,
                             getBatBitmap24(batPercent), 24, 24, dataColor);
  pos -= sp + 9;
#endif
  dataStr = String(getWiFidesc(rssi));
  drawString(pos, DISP_HEIGHT - 1 - 2, dataStr, RIGHT, dataColor);
  pos -= getStringWidth(dataStr) + 19;
  display.drawInvertedBitmap(pos, DISP_HEIGHT - 1 - 13, getWiFiBitmap16(rssi),
                             16, 16, dataColor);
  pos -= sp + 8;
  drawString(pos, DISP_HEIGHT - 1 - 2, refreshTimeStr, RIGHT, dataColor);
  pos -= getStringWidth(refreshTimeStr) + 25;
  display.drawInvertedBitmap(pos, DISP_HEIGHT - 1 - 21, wi_refresh_32x32,
                             32, 32, dataColor);
  pos -= sp;
  if (!statusStr.isEmpty())
  {
    drawString(pos, DISP_HEIGHT - 1 - 2, statusStr, RIGHT, ACCENT_COLOR);
    pos -= getStringWidth(statusStr) + 24;
    display.drawInvertedBitmap(pos, DISP_HEIGHT - 1 - 18, error_icon_24x24,
                               24, 24, ACCENT_COLOR);
  }
}

/* 绘制错误界面 */
void drawError(const uint8_t *bitmap_196x196,
               const String &errMsgLn1, const String &errMsgLn2)
{
  display.setFont(&FONT_26pt8b);
  if (!errMsgLn2.isEmpty())
  {
    drawString(DISP_WIDTH / 2, DISP_HEIGHT / 2 + 196 / 2 + 21,
               errMsgLn1, CENTER);
    drawString(DISP_WIDTH / 2, DISP_HEIGHT / 2 + 196 / 2 + 21 + 55,
               errMsgLn2, CENTER);
  }
  else
  {
    drawMultiLnString(DISP_WIDTH / 2,
                      DISP_HEIGHT / 2 + 196 / 2 + 21,
                      errMsgLn1, CENTER, DISP_WIDTH - 200, 2, 55);
  }
  display.drawInvertedBitmap(DISP_WIDTH / 2 - 196 / 2,
                             DISP_HEIGHT / 2 - 196 / 2 - 21,
                             bitmap_196x196, 196, 196, ACCENT_COLOR);
}
