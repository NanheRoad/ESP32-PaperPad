/* 简化渲染器：根据中国气象台数据在墨水屏上绘制内容 */
#include "_locale.h"
#include "_strftime.h"
#include "renderer.h"
#include "display_utils.h"
#include "config.h"

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

/* 计算字符串宽度 */
uint16_t getStringWidth(const String &text)
{
  int16_t x1, y1; uint16_t w, h;
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  return w;
}

/* 计算字符串高度 */
uint16_t getStringHeight(const String &text)
{
  int16_t x1, y1; uint16_t w, h;
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  return h;
}

/* 按对齐方式绘制字符串 */
void drawString(int16_t x, int16_t y, const String &text, alignment_t align,
                uint16_t color)
{
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
    int16_t x1, y1; uint16_t w, h;
    display.getTextBounds(remain, 0, 0, &x1, &y1, &w, &h);
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
          display.getTextBounds(sub + "...", 0, 0, &x1, &y1, &w, &h);
          if (w <= max_w) sub += "...";
        }
        else
        {
          display.getTextBounds(sub, 0, 0, &x1, &y1, &w, &h);
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
  pinMode(PIN_EPD_PWR, OUTPUT);
  digitalWrite(PIN_EPD_PWR, HIGH);
#ifdef DRIVER_WAVESHARE
  display.init(115200, true, 2, false);
#endif
#ifdef DRIVER_DESPI_C02
  display.init(115200, true, 10, false);
#endif
  SPI.end();
  SPI.begin(PIN_EPD_SCK, PIN_EPD_MISO, PIN_EPD_MOSI, PIN_EPD_CS);
  display.setRotation(0);
  display.setTextSize(1);
  display.setTextColor(GxEPD_BLACK);
  display.setTextWrap(false);
  display.setFullWindow();
  display.firstPage();
}

/* 关闭墨水屏电源 */
void powerOffDisplay()
{
  display.hibernate();
  digitalWrite(PIN_EPD_PWR, LOW);
}

/* 绘制当前天气和室内传感器数据 */
void drawCurrentWeather(const cma_weather_t &w,
                        float inTemp, float inHumidity)
{
  display.setFont(&FONT_26pt8b);
  drawString(10, 40, w.weather1 + "/" + w.weather2, LEFT);
  display.setFont(&FONT_16pt8b);
  drawString(10, 80, String("温度 ") + String(w.temperature,1) + "°C  湿度 " +
                      String(w.humidity) + "%", LEFT);
  drawString(10, 110, String("风 ") + w.windDirection + " " +
                      String(w.windSpeed,1) + "m/s", LEFT);
  drawString(10, 140, String("降水 ") + String(w.precipitation,1) + "mm", LEFT);
  drawString(10, 170, String("室内温度 ") + String(inTemp,1) + "°C  室内湿度 " +
                      String(inHumidity,1) + "%", LEFT);
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

