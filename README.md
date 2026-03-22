# ESP32 7.5 寸墨水屏天气站

这是一个基于 ESP32 的低功耗天气显示项目，使用 7.5 寸电子墨水屏显示中国气象台天气数据，并通过 SHT30 显示室内温湿度。设备支持 WiFi 联网、NTP 校时、外部 RTC 回退、电池电压检测和深度睡眠。

项目当前的软件来源于 `esp32-weather-epd` 的裁剪分支，已经切换到中国气象台接口与中文界面，但仍保留了部分上游默认配置和未清理逻辑。使用前建议先看完本文的“已知问题”。

## 项目结构

- `src/main.cpp`：主流程，负责启动、联网、校时、拉取天气、读取传感器、刷新显示、进入深度睡眠。
- `src/client_utils.cpp`：WiFi、SNTP、HTTP/HTTPS 请求。
- `src/api_response.cpp`：中国气象台 API JSON 解析。
- `src/renderer.cpp`：墨水屏初始化与页面绘制。
- `src/display_utils.cpp`：电池、电量、时间格式化、状态文本等工具函数。
- `src/config.cpp`：引脚、WiFi、API、时区、休眠、电池阈值等运行参数。
- `include/config.h`：编译期开关，包含屏幕型号、驱动模式、语言、单位、协议等宏。
- `docs/SCH_7.5寸墨水屏_2025-08-17.pdf`：当前项目原理图。

## 硬件分析

根据 [`docs/SCH_7.5寸墨水屏_2025-08-17.pdf`](/Users/n/Code/esp32-weather-epd/platformio/docs/SCH_7.5寸墨水屏_2025-08-17.pdf)，整机由以下模块组成：

- `ESP32-WROOM-32` 主控。
- `CH340C` USB 转串口，用于下载和调试。
- 锂电池充电与 3.3V 供电部分，包含 `TP4056` 充电管理和 `ME6211` LDO。
- `BL8025T` RTC，代码中使用 `PCF8563_Library` 兼容驱动。
- `SHT30-DIS-B` 温湿度传感器。
- 7.5 寸墨水屏 FPC 接口与升压/负压驱动电路。
- `KEY1/KEY2/KEY3`、`EN`、`IO0` 按键与上拉网络。

### 原理图里可以确认的主连线

按原理图网络名，核心信号如下：

| 功能 | 原理图网络 | ESP32 GPIO |
| --- | --- | --- |
| 电池电压采样 | `ADC_BAT` | GPIO36 |
| I2C 时钟 | `SCL` | GPIO22 |
| I2C 数据 | `SDA` | GPIO21 |
| 墨水屏数据 | `EPD_DIN` | GPIO23 |
| 墨水屏时钟 | `EPD_CLK` | GPIO18 |
| 墨水屏片选 | `EPD_CS` | GPIO5 |
| 墨水屏数据/命令 | `EPD_DC` | GPIO19 |
| 墨水屏忙信号 | `EPD_BUSY` | GPIO17 |
| 墨水屏复位 | `EPD_RST` | GPIO16 |
| 唤醒/控制 | `WAKE_IO` | GPIO26 |

这份原理图对应的是“板载墨水屏驱动电路”方案，不是外接现成 DESPI-C02 模块的接线图。

## 软件流程

程序启动后执行顺序如下：

1. 初始化串口、I2C 和外部 RTC。
2. 读取电池电压；电压过低时显示错误页并进入深度睡眠。
3. 连接 WiFi。
4. 使用 NTP 校时；若失败则回退到外部 RTC。
5. 通过 `cn.apihz.cn` 请求天气数据。
6. 读取 `SHT30` 室内温湿度。
7. 刷新墨水屏。
8. 按 `SLEEP_DURATION` 对齐后进入深度睡眠。

## 当前配置入口

需要首先检查的文件：

- [`src/config.cpp`](/Users/n/Code/esp32-weather-epd/platformio/src/config.cpp)：WiFi、API 账号、省市区县、城市显示名、时区、睡眠周期、电池阈值、引脚。
- [`include/config.h`](/Users/n/Code/esp32-weather-epd/platformio/include/config.h)：屏幕型号、驱动模式、语言、单位、HTTPS 模式、调试等级。

最少需要改的参数：

- `WIFI_SSID`
- `WIFI_PASSWORD`
- `CMA_PID`
- `CMA_KEY`
- `CMA_PROVINCE`
- `CMA_CITY`
- `CMA_PLACE`
- `CITY_STRING`
- `TIMEZONE`

## 已知问题

1. [`include/cert.h`](/Users/n/Code/esp32-weather-epd/platformio/include/cert.h) 的注释和生成信息仍指向 `api.openweathermap.org`，不是当前的 `cn.apihz.cn`。虽然当前配置使用的是 Sectigo CA，短期内可工作，但证书维护信息已经过期，建议后续按当前域名重新生成证书文件。

## 构建与烧录

项目使用 PlatformIO。

1. 安装 VS Code 和 PlatformIO 插件。
2. 打开本目录：`platformio/`
3. 根据硬件修正 [`src/config.cpp`](/Users/n/Code/esp32-weather-epd/platformio/src/config.cpp) 和 [`include/config.h`](/Users/n/Code/esp32-weather-epd/platformio/include/config.h)。
4. 连接 ESP32 开发板。
5. 执行构建和烧录。

如果本机装了 PlatformIO CLI，也可以使用：

```bash
pio run
pio run -t upload
```

## 说明

- 当前工作区没有可用的 `pio` 命令，因此这次只完成了静态分析，没有完成本地编译验证。
- 仓库里原有 `README.md` 和 `README_zh.md` 之前已被删除；本文件是基于当前代码和原理图重新整理的项目说明。
