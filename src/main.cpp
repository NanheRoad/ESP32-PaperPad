/* esp32-weather-epd 主程序
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
#include <Adafruit_SHT31.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <pcf8563.h>
#include <Preferences.h>
#include <algorithm>
#include <time.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <qrcode.h>
#include "_locale.h"
#include "api_response.h"
#include "client_utils.h"
#include "config.h"
#include "display_utils.h"
#include "icons/icons_196x196.h"
#include "renderer.h"
#if defined(USE_HTTPS_NO_CERT_VERIF) || defined(USE_HTTPS_WITH_CERT_VERIF)
  #include <WiFiClientSecure.h>
#endif
#ifdef USE_HTTPS_WITH_CERT_VERIF
  #include "cert.h"
#endif

// 太大，无法在栈上分配
static cma_weather_t weather_data;
// 使用 lewisxhe/PCF8563_Library 驱动 BL8025C 实时时钟
static PCF8563_Class rtc; // 外部 RTC

Preferences prefs;

static const unsigned long WIFI_CONNECT_TIMEOUT_MS = 12000;
static const unsigned long PORTAL_TIMEOUT_MS = 300000; // 配网等待时间
static const char *PORTAL_NAMESPACE = NVS_NAMESPACE;
static const char *NVS_WIFI_SSID = "wifi_ssid";
static const char *NVS_WIFI_PASS = "wifi_pass";
static const char *NVS_NTP1 = "ntp1";
static const char *NVS_NTP2 = "ntp2";
static const char *NVS_PREVIEW_API_HOST = "preview_api";
static const uint8_t KEY3_ACTIVE_LEVEL = LOW; // 按键按下时电平（默认低电平）
static const uint32_t KEY3_LONG_PRESS_MS = 1500;

struct RuntimeNetConfig
{
  String ssid;
  String password;
  String ntp1;
  String ntp2;
  String previewApiHost;
};

static String htmlEscape(const String &in)
{
  String out;
  out.reserve(in.length() + 16);
  for (size_t i = 0; i < in.length(); ++i)
  {
    const char c = in[i];
    switch (c)
    {
      case '&': out += "&amp;"; break;
      case '<': out += "&lt;"; break;
      case '>': out += "&gt;"; break;
      case '"': out += "&quot;"; break;
      case '\'': out += "&#39;"; break;
      default: out += c; break;
    }
  }
  return out;
}

static RuntimeNetConfig loadRuntimeNetConfig(Preferences &p)
{
  RuntimeNetConfig cfg;
  cfg.ssid = p.getString(NVS_WIFI_SSID, WIFI_SSID);
  cfg.password = p.getString(NVS_WIFI_PASS, WIFI_PASSWORD);
  cfg.ntp1 = p.getString(NVS_NTP1, NTP_SERVER_1);
  cfg.ntp2 = p.getString(NVS_NTP2, NTP_SERVER_2);
  cfg.previewApiHost = p.getString(NVS_PREVIEW_API_HOST, "");
  cfg.ssid.trim();
  cfg.ntp1.trim();
  cfg.ntp2.trim();
  cfg.previewApiHost.trim();
  if (cfg.ssid.isEmpty()) cfg.ssid = WIFI_SSID;
  if (cfg.ntp1.isEmpty()) cfg.ntp1 = NTP_SERVER_1;
  if (cfg.ntp2.isEmpty()) cfg.ntp2 = NTP_SERVER_2;
  return cfg;
}

static void saveRuntimeNetConfig(const RuntimeNetConfig &cfg)
{
  Preferences p;
  p.begin(PORTAL_NAMESPACE, false);
  p.putString(NVS_WIFI_SSID, cfg.ssid);
  p.putString(NVS_WIFI_PASS, cfg.password);
  p.putString(NVS_NTP1, cfg.ntp1);
  p.putString(NVS_NTP2, cfg.ntp2);
  p.putString(NVS_PREVIEW_API_HOST, cfg.previewApiHost);
  p.end();
}

static bool isIPv4String(const String &s)
{
  IPAddress ip;
  return ip.fromString(s);
}

static bool isKey3LongPressedAtBoot()
{
  if (PIN_KEY3 < 0) return false;
  pinMode(PIN_KEY3, INPUT);
  if (digitalRead(PIN_KEY3) != KEY3_ACTIVE_LEVEL) return false;
  Serial.println("KEY3 检测：检测到按下，开始长按计时...");
  const unsigned long deadline = millis() + KEY3_LONG_PRESS_MS;
  while (millis() < deadline)
  {
    if (digitalRead(PIN_KEY3) != KEY3_ACTIVE_LEVEL)
    {
      Serial.println("KEY3 检测：未达到长按时长，忽略。");
      return false;
    }
    delay(10);
  }
  Serial.println("KEY3 检测：长按成立。");
  return true;
}

static void enableKey3Wakeup()
{
  if (PIN_KEY3 < 0) return;
  const uint64_t mask = (1ULL << static_cast<uint8_t>(PIN_KEY3));
  const auto mode = (KEY3_ACTIVE_LEVEL == LOW)
                  ? ESP_EXT1_WAKEUP_ALL_LOW
                  : ESP_EXT1_WAKEUP_ANY_HIGH;
  const esp_err_t err = esp_sleep_enable_ext1_wakeup(mask, mode);
  if (err == ESP_OK)
  {
    Serial.println(String("已启用 KEY3 深睡唤醒: GPIO") + String(PIN_KEY3)
                 + ", active=" + (KEY3_ACTIVE_LEVEL == LOW ? "LOW" : "HIGH"));
  }
  else
  {
    Serial.println("KEY3 深睡唤醒配置失败，错误码: " + String(static_cast<int>(err)));
  }
}

static bool wokeFromKey3()
{
  const auto cause = esp_sleep_get_wakeup_cause();
  if (cause == ESP_SLEEP_WAKEUP_EXT0 || cause == ESP_SLEEP_WAKEUP_EXT1)
  {
    Serial.println(String("唤醒原因：") + (cause == ESP_SLEEP_WAKEUP_EXT0 ? "EXT0" : "EXT1") + " (KEY3)");
    return true;
  }
  return false;
}

static String buildPortalHtml(const RuntimeNetConfig &cfg,
                              const String &apSsid,
                              const String &apPass,
                              const String &portalUrl)
{
  String html;
  html.reserve(4096);
  html += "<!doctype html><html><head><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<title>设备配网</title><style>";
  html += "body{font-family:-apple-system,BlinkMacSystemFont,'PingFang SC','Microsoft YaHei',sans-serif;";
  html += "margin:0;padding:16px;background:#f4f6f8;color:#1f2937}";
  html += ".card{max-width:560px;margin:0 auto;background:#fff;border-radius:12px;padding:16px;";
  html += "box-shadow:0 2px 12px rgba(0,0,0,.08)}h1{font-size:20px;margin:0 0 10px}";
  html += ".tip{font-size:14px;color:#4b5563;margin-bottom:14px;line-height:1.5}";
  html += "label{display:block;font-size:14px;margin:12px 0 6px}";
  html += "input{width:100%;box-sizing:border-box;border:1px solid #d1d5db;border-radius:10px;";
  html += "padding:10px 12px;font-size:16px}button{width:100%;margin-top:16px;border:none;";
  html += "border-radius:10px;padding:12px;background:#0f766e;color:#fff;font-size:16px}";
  html += ".busy{margin-top:12px;font-size:14px;color:#0f766e;display:none}";
  html += ".meta{font-size:13px;color:#6b7280;margin-top:10px;line-height:1.4}";
  html += "</style></head><body><div class='card'><h1>设备配网</h1>";
  html += "<div class='tip'>请填写 Wi-Fi、时间服务器与预览 API 地址（IPv4），保存后设备会立即联网校验。</div>";
  html += "<form method='POST' action='/save' onsubmit='return onSubmitCfg(this)'>";
  html += "<label>Wi-Fi 名称 (SSID)</label><input name='ssid' maxlength='64' value='";
  html += htmlEscape(cfg.ssid);
  html += "' required>";
  html += "<label>Wi-Fi 密码</label><input name='password' type='password' maxlength='64' value='";
  html += htmlEscape(cfg.password);
  html += "'>";
  html += "<label>NTP 服务器 1</label><input name='ntp1' maxlength='64' value='";
  html += htmlEscape(cfg.ntp1);
  html += "' required>";
  html += "<label>NTP 服务器 2</label><input name='ntp2' maxlength='64' value='";
  html += htmlEscape(cfg.ntp2);
  html += "' required>";
  html += "<label>预览 API 地址（IPv4）</label><input name='preview_api' maxlength='15' value='";
  html += htmlEscape(cfg.previewApiHost);
  html += "' required>";
  html += "<button id='submitBtn' type='submit'>确定并连接</button>";
  html += "<div id='busyTip' class='busy'>正在提交并验证连接，请稍候...</div></form>";
  html += "<div class='meta'>配网热点: ";
  html += htmlEscape(apSsid);
  html += " / 密码: ";
  html += htmlEscape(apPass);
  html += "<br>配网地址: ";
  html += htmlEscape(portalUrl);
  html += "</div></div>";
  html += "<script>";
  html += "function onSubmitCfg(form){";
  html += "var btn=document.getElementById('submitBtn');";
  html += "var tip=document.getElementById('busyTip');";
  html += "if(btn){btn.disabled=true;btn.innerText='提交中...';btn.style.opacity='0.75';}";
  html += "if(tip){tip.style.display='block';}";
  html += "return true;}";
  html += "</script></body></html>";
  return html;
}

static void drawProvisionQr(const String &apSsid,
                            const String &apPass,
                            const String &portalUrl)
{
  const uint8_t qrVersion = 3;
  uint8_t qrData[qrcode_getBufferSize(qrVersion)];
  QRCode qr;
  qrcode_initText(&qr, qrData, qrVersion, 0, portalUrl.c_str());

  const int qrSize = qr.size;
  const int target = 210;
  const int scale = std::max(2, target / qrSize);
  const int pixelSize = qrSize * scale;
  const int x0 = 24;
  const int y0 = 66;

  initDisplay();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    drawString(12, 34, "联网失败，进入扫码配网", LEFT, GxEPD_BLACK);
    drawString(12, 54, "请先连接设备热点后再扫码", LEFT, GxEPD_BLACK);
    display.drawRect(x0 - 4, y0 - 4, pixelSize + 8, pixelSize + 8, GxEPD_BLACK);
    for (int y = 0; y < qrSize; ++y)
    {
      for (int x = 0; x < qrSize; ++x)
      {
        if (qrcode_getModule(&qr, x, y))
        {
          display.fillRect(x0 + x * scale, y0 + y * scale, scale, scale, GxEPD_BLACK);
        }
      }
    }
    drawString(270, 96, "热点: " + apSsid, LEFT, GxEPD_BLACK);
    drawString(270, 126, "密码: " + apPass, LEFT, GxEPD_BLACK);
    drawString(270, 156, "地址: " + portalUrl, LEFT, GxEPD_BLACK);
    drawString(270, 186, "页面可设置 Wi-Fi 与 NTP", LEFT, GxEPD_BLACK);
    drawString(270, 216, "超时未配置将自动休眠", LEFT, ACCENT_COLOR);
  } while (display.nextPage());
  powerOffDisplay();
}

static void drawPortalTimeoutAndSleep()
{
  initDisplay();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    drawString(DISP_WIDTH / 2, DISP_HEIGHT / 2 - 10, "配网超时，无人操作", CENTER, ACCENT_COLOR);
    drawString(DISP_WIDTH / 2, DISP_HEIGHT / 2 + 22, "二维码已清除，设备即将休眠", CENTER, GxEPD_BLACK);
  } while (display.nextPage());
  powerOffDisplay();
}

static bool runProvisionPortal(RuntimeNetConfig &cfg)
{
  uint32_t chip = static_cast<uint32_t>(ESP.getEfuseMac() & 0xFFFFFF);
  String apSsid = "EPD-Setup-" + String(chip, HEX);
  apSsid.toUpperCase();
  const String apPass = "12345678";

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(apSsid.c_str(), apPass.c_str());
  delay(100);
  IPAddress apIp = WiFi.softAPIP();
  const String portalUrl = "http://" + apIp.toString();

  drawProvisionQr(apSsid, apPass, portalUrl);
  Serial.println("配网 AP 已启动: " + apSsid + " / " + apIp.toString());

  DNSServer dns;
  dns.start(53, "*", apIp);
  WebServer server(80);
  bool saveOk = false;
  unsigned long exitAfterMs = 0;

  server.on("/", HTTP_GET, [&]()
  {
    server.send(200, "text/html; charset=utf-8", buildPortalHtml(cfg, apSsid, apPass, portalUrl));
  });

  server.on("/save", HTTP_POST, [&]()
  {
    RuntimeNetConfig newCfg = cfg;
    newCfg.ssid = server.arg("ssid");
    newCfg.password = server.arg("password");
    newCfg.ntp1 = server.arg("ntp1");
    newCfg.ntp2 = server.arg("ntp2");
    newCfg.previewApiHost = server.arg("preview_api");
    newCfg.ssid.trim();
    newCfg.ntp1.trim();
    newCfg.ntp2.trim();
    newCfg.previewApiHost.trim();
    if (newCfg.ssid.isEmpty() || newCfg.ntp1.isEmpty() || newCfg.ntp2.isEmpty()
        || newCfg.previewApiHost.isEmpty())
    {
      server.send(400, "text/html; charset=utf-8",
                  "<html><body><h3>参数不完整</h3><p>SSID、NTP、预览 API 地址都不能为空。</p>"
                  "<p><a href='/'>返回</a></p></body></html>");
      return;
    }
    if (!isIPv4String(newCfg.previewApiHost))
    {
      server.send(400, "text/html; charset=utf-8",
                  "<html><body><h3>预览 API 地址格式错误</h3><p>请填写 IPv4，例如 192.168.1.10。</p>"
                  "<p><a href='/'>返回重试</a></p></body></html>");
      return;
    }

    Serial.println("收到配网请求，正在验证 Wi-Fi...");
    WiFi.begin(newCfg.ssid.c_str(), newCfg.password.c_str());
    const unsigned long deadline = millis() + WIFI_CONNECT_TIMEOUT_MS;
    while (WiFi.status() != WL_CONNECTED && millis() < deadline)
    {
      delay(100);
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      cfg = newCfg;
      saveRuntimeNetConfig(cfg);
      saveOk = true;
      server.send(200, "text/html; charset=utf-8",
                  "<!doctype html><html><head><meta charset='utf-8'>"
                  "<meta name='viewport' content='width=device-width,initial-scale=1'>"
                  "<style>body{font-family:-apple-system,BlinkMacSystemFont,'PingFang SC','Microsoft YaHei',sans-serif;"
                  "background:#f4f6f8;margin:0;padding:24px;color:#1f2937}"
                  ".card{max-width:520px;margin:0 auto;background:#fff;border-radius:12px;padding:20px;"
                  "box-shadow:0 2px 12px rgba(0,0,0,.08);text-align:center}"
                  ".ok{font-size:42px;color:#0f766e;font-weight:700;margin:8px 0 6px}"
                  ".btn{display:inline-block;margin-top:14px;background:#0f766e;color:#fff;text-decoration:none;"
                  "padding:10px 24px;border-radius:10px}</style></head><body>"
                  "<div class='card'><div class='ok'>OK</div><h3>配网成功</h3>"
                  "<p>设备已联网并保存配置，可关闭本页面。</p>"
                  "<a class='btn' href='#' onclick='window.close();return false;'>确定</a></div></body></html>");
      exitAfterMs = millis() + 3500;
      Serial.println("配网成功，已保存到 NVS。");
      return;
    }

    server.send(400, "text/html; charset=utf-8",
                "<html><body><h3>连接失败</h3><p>请检查 SSID/密码后重试。</p>"
                "<p><a href='/'>返回重试</a></p></body></html>");
    Serial.println("配网失败：Wi-Fi 连接测试未通过。");
  });

  server.begin();
  const unsigned long deadline = millis() + PORTAL_TIMEOUT_MS;
  while (millis() < deadline)
  {
    dns.processNextRequest();
    server.handleClient();
    if (saveOk && exitAfterMs > 0 && millis() >= exitAfterMs) break;
    delay(5);
  }

  server.stop();
  dns.stop();
  WiFi.softAPdisconnect(true);

  if (!saveOk)
  {
    killWiFi();
  }
  return saveOk;
}

static bool hasDefaultApiConfig()
{
  return CMA_PID == "your_id"
      || CMA_KEY == "your_key"
      || CMA_PROVINCE == "省份"
      || CMA_CITY == "城市"
      || CMA_PLACE == "区县";
}

#if DISPLAY_SELF_TEST
static void runDisplaySelfTest()
{
  Serial.println("进入显示自检模式：仅测试墨水屏，不联网。");
  pinMode(PIN_EPD_BUSY, INPUT);
  Serial.println("BUSY(init前) = " + String(digitalRead(PIN_EPD_BUSY)));
  initDisplay();
  Serial.println("BUSY(init后) = " + String(digitalRead(PIN_EPD_BUSY)));
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.fillRect(0, 0, DISP_WIDTH / 3, DISP_HEIGHT, GxEPD_BLACK);
#if defined(DISP_3C_B) || defined(DISP_7C_F)
    display.fillRect(DISP_WIDTH / 3, 0, DISP_WIDTH / 3, DISP_HEIGHT, ACCENT_COLOR);
#endif
    drawString(DISP_WIDTH - 16, 48, "EPD SELF TEST", RIGHT, GxEPD_BLACK);
    drawString(DISP_WIDTH - 16, 82, "BUSY/RST/DC/CS/SPI", RIGHT, GxEPD_BLACK);
    drawString(DISP_WIDTH - 16, 116, "If no update, check BUSY(17)", RIGHT, GxEPD_BLACK);
  } while (display.nextPage());
  Serial.println("BUSY(刷新后) = " + String(digitalRead(PIN_EPD_BUSY)));
  powerOffDisplay();
  Serial.println("BUSY(关机后) = " + String(digitalRead(PIN_EPD_BUSY)));
  Serial.println("显示自检完成，停止后续流程。");
}
#endif


/* 让 esp32 进入超低功耗深度睡眠（<11μA）。
 * 唤醒时间对齐到分钟。睡眠时间在 config.cpp 中定义。
 */
void beginDeepSleep(unsigned long startTime, tm *timeInfo)
{
  if (!getLocalTime(timeInfo))
  {
    Serial.println(TXT_REFERENCING_OLDER_TIME_NOTICE);
  }

  // 为简化睡眠时间计算，当前由 timeInfo 存储的时间将被转换为相对于 WAKE_TIME 的时间。
  // 这样，如果 SLEEP_DURATION 不是 60 分钟的倍数，可以更容易地对齐，
  // 并且可以轻松判断是否需要因睡觉时间而额外睡眠。
  // 例如，当 curHour == 0 时，timeInfo->tm_hour == WAKE_TIME
  int bedtimeHour = INT_MAX;
  if (BED_TIME != WAKE_TIME)
  {
    bedtimeHour = (BED_TIME - WAKE_TIME + 24) % 24;
  }

  // 时间相对于唤醒时间
  int curHour = (timeInfo->tm_hour - WAKE_TIME + 24) % 24;
  const int curMinute = curHour * 60 + timeInfo->tm_min;
  const int curSecond = curHour * 3600
                      + timeInfo->tm_min * 60
                      + timeInfo->tm_sec;
  const int desiredSleepSeconds = SLEEP_DURATION * 60;
  const int offsetMinutes = curMinute % SLEEP_DURATION;
  const int offsetSeconds = curSecond % desiredSleepSeconds;

  // 唤醒时间对齐到 SLEEP_DURATION 的最近倍数
  int sleepMinutes = SLEEP_DURATION - offsetMinutes;
  if (desiredSleepSeconds - offsetSeconds < 120
   || offsetSeconds / (float)desiredSleepSeconds > 0.95f)
  { // 如果睡眠时间少于 2 分钟或少于 SLEEP_DURATION 的 5%，则跳到下一个对齐点
    sleepMinutes += SLEEP_DURATION;
  }

  // 预计唤醒时间，如果落在睡眠区间则需要调整 sleepDuration
  const int predictedWakeHour = ((curMinute + sleepMinutes) / 60) % 24;

  uint64_t sleepDuration;
  if (predictedWakeHour < bedtimeHour)
  {
    sleepDuration = sleepMinutes * 60 - timeInfo->tm_sec;
  }
  else
  {
    const int hoursUntilWake = 24 - curHour;
    sleepDuration = hoursUntilWake * 3600ULL
                    - (timeInfo->tm_min * 60ULL + timeInfo->tm_sec);
  }

  // 增加额外延迟以补偿部分 esp32 RTC 过快的问题
  sleepDuration += 3ULL;
  sleepDuration *= 1.0015f;

#if DEBUG_LEVEL >= 1
  printHeapUsage();
#endif

  esp_sleep_enable_timer_wakeup(sleepDuration * 1000000ULL);
  enableKey3Wakeup();
  Serial.print(TXT_AWAKE_FOR);
  Serial.println(" "  + String((millis() - startTime) / 1000.0, 3) + "秒");
  Serial.print(TXT_ENTERING_DEEP_SLEEP_FOR);
  Serial.println(" " + String(sleepDuration) + "秒");
  esp_deep_sleep_start();
} // end beginDeepSleep

/* 程序入口
 */
void setup()
{
  unsigned long startTime = millis();
  Serial.begin(115200);
  delay(20);
  Serial.println("启动: wake_cause=" + String(static_cast<int>(esp_sleep_get_wakeup_cause())));
  if (PIN_KEY3 >= 0)
  {
    pinMode(PIN_KEY3, INPUT);
    Serial.println("启动: KEY3 GPIO" + String(PIN_KEY3) + " 电平=" + String(digitalRead(PIN_KEY3)));
  }

  // 初始化 I2C 与外部实时时钟
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  rtc.begin();

#if DEBUG_LEVEL >= 1
  printHeapUsage();
#endif

  disableBuiltinLED();

#if DISPLAY_SELF_TEST
  runDisplaySelfTest();
  return;
#endif

  // 打开命名空间以读写非易失性存储
  prefs.begin(NVS_NAMESPACE, false);

#if BATTERY_MONITORING
  uint32_t batteryVoltage = readBatteryVoltage();
  Serial.print(TXT_BATTERY_VOLTAGE);
  Serial.println("：" + String(batteryVoltage) + "毫伏");

  // USB-only 场景下（未接电池）ADC_BAT 可能悬空，读数会异常偏低。
  // 这类无效值不应触发低电保护，否则设备会开机即休眠。
  const bool batteryReadingInvalid = (batteryVoltage < 1000);
  if (batteryReadingInvalid)
  {
    Serial.println("电池电压读数无效，跳过低电保护（可能为USB供电且未接电池）");
    batteryVoltage = UINT32_MAX;
  }

  // 当电池电量低时，应该刷新显示，但只在首次检测到低电压时刷新。
  // 下次刷新是在电压恢复正常时。为此我们会用非易失性存储做标记。
  bool lowBat = prefs.getBool("lowBat", false);

  // 电池电量低，立即进入深度睡眠
  if (!batteryReadingInvalid && batteryVoltage <= LOW_BATTERY_VOLTAGE)
  {
    if (lowBat == false)
    {  // 首次检测到电池电量低
      prefs.putBool("lowBat", true);
      prefs.end();
      initDisplay();
      do
      {
        drawError(battery_alert_0deg_196x196, TXT_LOW_BATTERY);
      } while (display.nextPage());
      powerOffDisplay();
    }

    if (batteryVoltage <= CRIT_LOW_BATTERY_VOLTAGE)
    { // 严重低电压
      // 不设置 esp_sleep_enable_timer_wakeup();
      // 可由 KEY3 外部唤醒或手动 RST 唤醒
      Serial.println(TXT_CRIT_LOW_BATTERY_VOLTAGE);
      Serial.println(TXT_HIBERNATING_INDEFINITELY_NOTICE);
    }
    else if (batteryVoltage <= VERY_LOW_BATTERY_VOLTAGE)
    { // 非常低电压
      esp_sleep_enable_timer_wakeup(VERY_LOW_BATTERY_SLEEP_INTERVAL
                                    * 60ULL * 1000000ULL);
      Serial.println(TXT_VERY_LOW_BATTERY_VOLTAGE);
      Serial.print(TXT_ENTERING_DEEP_SLEEP_FOR);
      Serial.println(" " + String(VERY_LOW_BATTERY_SLEEP_INTERVAL) + "分钟");
    }
    else
    {  // 低电压
      esp_sleep_enable_timer_wakeup(LOW_BATTERY_SLEEP_INTERVAL
                                    * 60ULL * 1000000ULL);
      Serial.println(TXT_LOW_BATTERY_VOLTAGE);
      Serial.print(TXT_ENTERING_DEEP_SLEEP_FOR);
      Serial.println(" " + String(LOW_BATTERY_SLEEP_INTERVAL) + "分钟");
    }
    enableKey3Wakeup();
    esp_deep_sleep_start();
  }
  // 电池恢复正常，重置非易失性存储变量
  if (lowBat == true)
  {
    prefs.putBool("lowBat", false);
  }
#else
  uint32_t batteryVoltage = UINT32_MAX;
#endif

  RuntimeNetConfig netCfg = loadRuntimeNetConfig(prefs);
  // 所有数据已从 NVS 加载，关闭文件系统
  prefs.end();

  String statusStr = {};
  String tmpStr = {};
  tm timeInfo = {};

  // START WIFI
  int wifiRSSI = 0; // “接收信号强度指示器"
  wl_status_t wifiStatus = WL_DISCONNECTED;
  const bool forcePortal = wokeFromKey3() || isKey3LongPressedAtBoot();
  if (forcePortal)
  {
    Serial.println("检测到 KEY3 长按触发，强制进入配网模式。");
  }
  else
  {
    wifiStatus = startWiFi(wifiRSSI, netCfg.ssid.c_str(), netCfg.password.c_str());
  }
  if (forcePortal || wifiStatus != WL_CONNECTED)
  { // WiFi 连接失败
    killWiFi();

    const bool configured = runProvisionPortal(netCfg);
    if (!configured)
    {
      drawPortalTimeoutAndSleep();
      beginDeepSleep(startTime, &timeInfo);
    }

    wifiStatus = WiFi.status();
    if (wifiStatus != WL_CONNECTED)
    {
      Serial.println("配网保存成功，但重连失败，进入休眠。");
      beginDeepSleep(startTime, &timeInfo);
    }
    wifiRSSI = WiFi.RSSI();
    Serial.println("配网后联网成功，继续执行主流程。");
  }

  // 时间同步
  configTzTime(TIMEZONE, netCfg.ntp1.c_str(), netCfg.ntp2.c_str());
  bool timeConfigured = waitForSNTPSync(&timeInfo);
  if (!timeConfigured)
  {
    Serial.println(TXT_TIME_SYNCHRONIZATION_FAILED);
    // 使用外部 RTC 时间作为回退
    auto now = rtc.getDateTime();
    timeInfo.tm_year = now.year - 1900;
    timeInfo.tm_mon  = now.month - 1;
    timeInfo.tm_mday = now.day;
    timeInfo.tm_hour = now.hour;
    timeInfo.tm_min  = now.minute;
    timeInfo.tm_sec  = now.second;
  }

  // API 请求
#ifdef USE_HTTP
  WiFiClient client;
#elif defined(USE_HTTPS_NO_CERT_VERIF)
  WiFiClientSecure client;
  client.setInsecure();
#elif defined(USE_HTTPS_WITH_CERT_VERIF)
  WiFiClientSecure client;
  client.setCACert(cert_Sectigo_RSA_Domain_Validation_Secure_Server_CA);
#endif
  if (hasDefaultApiConfig())
  {
    killWiFi();
    statusStr = "中国气象台 API";
    tmpStr = "请先配置 id/key/省市区";
    Serial.println("配置错误：请在 config.cpp 中填入真实 CMA id/key/省市区。");
    initDisplay();
    do
    {
      drawError(wi_cloud_down_196x196, statusStr, tmpStr);
    } while (display.nextPage());
    powerOffDisplay();
    beginDeepSleep(startTime, &timeInfo);
  }

  int rxStatus = getCMAweather(client, weather_data);
  if (rxStatus != HTTP_CODE_OK)
    {
      killWiFi();
      statusStr = "中国气象台 API";
      if (!weather_data.message.isEmpty())
      {
        tmpStr = String(rxStatus, DEC) + "：" + weather_data.message;
      }
      else
      {
        tmpStr = String(rxStatus, DEC) + "：" + getHttpResponsePhrase(rxStatus);
      }
    initDisplay();
    do
    {
      drawError(wi_cloud_down_196x196, statusStr, tmpStr);
    } while (display.nextPage());
    powerOffDisplay();
    beginDeepSleep(startTime, &timeInfo);
  }
    killWiFi();  // WiFi 不再需要

  // 读取室内温湿度，使用 SHT30 传感器
  float inTemp     = NAN;
  float inHumidity = NAN;
  Serial.print(String(TXT_READING_FROM) + " SHT30... ");
  Adafruit_SHT31 sht30 = Adafruit_SHT31();

  // Adafruit_SHT31 库当前版本仅支持全局 Wire 总线
  if (sht30.begin(SHT30_ADDRESS))
  {
    inTemp     = sht30.readTemperature(); // 摄氏度
    inHumidity = sht30.readHumidity();    // %

    // 检查 SHT30 读数是否有效
    if (std::isnan(inTemp) || std::isnan(inHumidity))
    {
      statusStr = "SHT30 " + String(TXT_READ_FAILED);
      Serial.println(statusStr);
    }
    else
    {
      Serial.println(TXT_SUCCESS);
    }
  }
  else
  {
    statusStr = "SHT30 " + String(TXT_NOT_FOUND); // 检查接线
    Serial.println(statusStr);
  }

  String refreshTimeStr;
  getRefreshTimeStr(refreshTimeStr, timeConfigured, &timeInfo);
  String dateStr;
  getDateStr(dateStr, &timeInfo);

  // 全屏刷新渲染
  initDisplay();
  do
  {
    drawCurrentWeather(weather_data, inTemp, inHumidity);
    drawLocationDate(CITY_STRING, dateStr);
    drawStatusBar(statusStr, refreshTimeStr, wifiRSSI, batteryVoltage);
  } while (display.nextPage());
  powerOffDisplay();

  // 深度睡眠
  beginDeepSleep(startTime, &timeInfo);
} // end setup

/* 永远不会运行
 */
void loop()
{
} // end loop
