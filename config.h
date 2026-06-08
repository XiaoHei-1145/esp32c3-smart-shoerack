#pragma once

#include <Arduino.h>
#ifdef ESP8266
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
#endif
#include <DHT.h>

// ==================== WiFi AP配置 ====================
extern const char* ap_ssid;
extern const char* ap_password;
extern IPAddress local_IP;
extern IPAddress gateway;
extern IPAddress subnet;

#ifdef ESP8266
  extern ESP8266WebServer server;
#elif defined(ESP32)
  extern WebServer server;
#endif

extern bool wifiConnected;

// ==================== 手动强制关闭标志 ====================
extern bool manualForceStopTemp;
extern bool manualForceStopHumidity;

// ==================== 引脚定义 ====================
#define BUZZER_PIN 12
#define D5_LED_PIN 13
#define TIMER_SWITCH_PIN 9
#define DHT11_PIN 10

#define UV_SWITCH_PIN 1
#define HEATER_FAN_SWITCH_PIN 3
#define FAN_ONLY_SWITCH_PIN 4

#define UV_OUTPUT_PIN 5
#define HEATER_OUTPUT_PIN 6
#define FAN_OUTPUT_PIN 7

// ==================== 温湿度控制参数 ====================
#define TEMP_HIGH_THRESHOLD 43
#define TEMP_LOW_THRESHOLD 35
#define HUMIDITY_HIGH_THRESHOLD 65
#define HUMIDITY_LOW_THRESHOLD 58

// ==================== 按键时间参数 ====================
#define SHORT_PRESS_MIN 20
#define SHORT_PRESS_MAX 280
#define LONG_PRESS_MIN 280
#define LONG_PRESS_MAX 600
#define ULTRA_LONG_PRESS 600

// ==================== 定时挡位定义 ====================
enum TimerMode {
  MODE_20_SECONDS = 0,
  MODE_30_MINUTES = 1,
  MODE_1_HOUR = 2,
  MODE_2_HOURS = 3
};

const unsigned long TIMER_DURATIONS[] = {
  20000,      // 20秒
  1800000,    // 30分钟
  3600000,    // 1小时
  7200000     // 2小时
};

const char* TIMER_MODE_NAMES[] = {
  "20秒",
  "30分钟",
  "1小时",
  "2小时"
};

const char* BUZZER_PATTERNS[] = {
  "10",
  "100",
  "1000",
  "10000",
  "11000"
};

extern TimerMode globalCurrentMode;

// ==================== DHT传感器 ====================
extern DHT dht;

// ==================== 蜂鸣器状态枚举 ====================
enum BuzzerState {
  BUZZER_IDLE,
  BUZZER_PLAYING,
  BUZZER_START_DELAY,
  BUZZER_START_BEEPING
};

extern BuzzerState buzzerState;
extern unsigned long buzzerTimer;
extern int currentBuzzerPattern;
extern int buzzerStepIndex;

// ==================== 功能状态类 ====================
class FunctionState {
public:
  unsigned long lastBtnTime = 0;
  unsigned long btnPressTime = 0;
  bool functionActive = false;
  unsigned long timerStart = 0;
  int btnState = 1;
  int lastBtnState = 1;
  bool waitingForRelease = false;
  TimerMode currentMode = MODE_20_SECONDS;
};

extern FunctionState uvState;
extern FunctionState heaterFanState;
extern FunctionState fanOnlyState;

// ==================== 温湿度自动控制状态 ====================
extern bool autoTempControl;
extern bool autoHumidityControl;
extern bool autoTempLocked;
extern bool autoHumidityLocked;
extern bool lastAutoTempControl;
extern bool lastAutoHumidityControl;

extern unsigned long lastSensorReadTime;
extern const unsigned long sensorReadInterval;
extern int sensorErrorCount;
extern const int maxSensorErrors;

extern float currentTemperature;
extern float currentHumidity;

// ==================== 定时调节按钮状态 ====================
extern int timerSwitchState;
extern int lastTimerSwitchState;
extern unsigned long lastTimerSwitchTime;
extern unsigned long timerSwitchPressTime;
extern bool timerSwitchWaitingForRelease;

// ==================== 其他状态标志 ====================
extern bool lastDeviceEnded;
extern bool checkEndBeepPlayed;
extern bool manualCancel;
