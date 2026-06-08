#include "config.h"
#include "buzzer.h"
#include "utils.h"
#include "controls.h"
#include "sensor.h"
#include "html_page.h"
#include "web.h"

// ==================== WiFi AP配置 ====================
const char* ap_ssid = "12123";
const char* ap_password = "123123";
IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

#ifdef ESP8266
  ESP8266WebServer server(80);
#elif defined(ESP32)
  WebServer server(80);
#endif

bool wifiConnected = false;

// ==================== 手动强制关闭标志 ====================
bool manualForceStopTemp = false;
bool manualForceStopHumidity = false;

// ==================== 当前全局挡位 ====================
TimerMode globalCurrentMode = MODE_20_SECONDS;

// ==================== DHT传感器 ====================
DHT dht(DHT11_PIN, DHT11);

// ==================== 功能状态对象 ====================
FunctionState uvState;
FunctionState heaterFanState;
FunctionState fanOnlyState;

// ==================== 温湿度自动控制状态 ====================
bool autoTempControl = false;
bool autoHumidityControl = false;
bool autoTempLocked = false;
bool autoHumidityLocked = false;
bool lastAutoTempControl = false;
bool lastAutoHumidityControl = false;

unsigned long lastSensorReadTime = 0;
const unsigned long sensorReadInterval = 5000;
int sensorErrorCount = 0;
const int maxSensorErrors = 3;

float currentTemperature = NAN;
float currentHumidity = NAN;

// ==================== 蜂鸣器状态 ====================
BuzzerState buzzerState = BUZZER_IDLE;
unsigned long buzzerTimer = 0;
int currentBuzzerPattern = 0;
int buzzerStepIndex = 0;

// ==================== 定时调节按钮状态 ====================
int timerSwitchState = 1;
int lastTimerSwitchState = 1;
unsigned long lastTimerSwitchTime = 0;
unsigned long timerSwitchPressTime = 0;
bool timerSwitchWaitingForRelease = false;

// ==================== 其他状态标志 ====================
bool lastDeviceEnded = false;
bool checkEndBeepPlayed = false;
bool manualCancel = false;

// ==================== Setup ====================
void setup() {
  Serial.begin(115200);
  delay(100);
  
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(D5_LED_PIN, OUTPUT);
  pinMode(TIMER_SWITCH_PIN, INPUT_PULLUP);
  pinMode(UV_SWITCH_PIN, INPUT_PULLUP);
  pinMode(HEATER_FAN_SWITCH_PIN, INPUT_PULLUP);
  pinMode(FAN_ONLY_SWITCH_PIN, INPUT_PULLUP);
  pinMode(UV_OUTPUT_PIN, OUTPUT);
  pinMode(HEATER_OUTPUT_PIN, OUTPUT);
  pinMode(FAN_OUTPUT_PIN, OUTPUT);
  
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(D5_LED_PIN, LOW);
  digitalWrite(UV_OUTPUT_PIN, LOW);
  digitalWrite(HEATER_OUTPUT_PIN, LOW);
  digitalWrite(FAN_OUTPUT_PIN, LOW);
  
  Serial.println("初始化DHT11传感器...");
  dht.begin();
  delay(2000);
  
  float testTemp = dht.readTemperature();
  float testHumidity = dht.readHumidity();
  
  if (isnan(testTemp) || isnan(testHumidity)) {
    Serial.println("❌ DHT11传感器初始化失败！");
    Serial.println("请检查：");
    Serial.println("1. 接线是否正确（VCC=3.3V, GND=GND, DATA=GPIO10）");
    Serial.println("2. DATA引脚是否接4.7K上拉电阻到3.3V");
    Serial.println("3. 传感器是否损坏");
  } else {
    Serial.println("✅ DHT11传感器初始化成功");
    Serial.print("测试读数: ");
    Serial.print(testTemp);
    Serial.print("°C, ");
    Serial.print(testHumidity);
    Serial.println("%");
    currentTemperature = testTemp;
    currentHumidity = testHumidity;
  }
  
  Serial.println("\n📶 初始化WiFi AP模式...");
  createWiFiAP();
  
  server.on("/", handleRoot);
  server.on("/control", handleControl);
  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("✅ Web服务器已启动");
  
  Serial.println("\n🌡️ 温湿度控制系统启动完成");
  Serial.println("⏰ 定时模式: 20秒、30分钟、1小时、2小时");
  Serial.println("🔘 定时调节按钮: GPIO9 (安全引脚，使用内部上拉)");
  Serial.println("📢 蜂鸣器: GPIO12");
  Serial.println("🎵 摩斯码: 1=0.6秒, 0=0.1秒, 间隔=0.4秒");
  Serial.println("🎵 启动音: 延迟0.2秒后响0.1秒 (优先级最高)");
  Serial.println("📝 功能: 每个设备独立记录启动时的挡位");
  Serial.println("📝 功能: 切换挡位不影响已运行设备");
  Serial.println("📝 功能: 仅定时结束或自动启停播放结束音");
  Serial.println("📝 功能: 温湿度自动控制也会触发蜂鸣器");
  Serial.println("🔥 温度控制: >43℃开启风扇, <35℃关闭风扇");
  Serial.println("💧 湿度控制: >65%开启加热+风扇, <58%关闭加热+风扇");
  Serial.println("🔒 长按可强制关闭自动设备并锁定自动控制");
  Serial.println("💡 D5灯: 设备运行时亮起");
  Serial.println("🔘 功能按键时间: 短按20-280ms, 长按280-600ms");
  Serial.println("⏰ 超长按>600ms直接执行并等待释放");
  Serial.println("📱 WiFi控制: 手机连接 '12123' (密码123123)，访问 http://192.168.4.1");
  Serial.println("Smart Desk Controllerxh (c) 2026");
  
  playBuzzerPattern(globalCurrentMode);
}

// ==================== Loop ====================
void loop() {
  if (wifiConnected) {
    server.handleClient();
  }
  
  checkTimerSwitch();
  
  checkSwitch(UV_SWITCH_PIN, uvState, UV_OUTPUT_PIN, "紫外线开关");
  checkSwitch(HEATER_FAN_SWITCH_PIN, heaterFanState, HEATER_OUTPUT_PIN, "加热+风扇开关", true);
  checkSwitch(FAN_ONLY_SWITCH_PIN, fanOnlyState, FAN_OUTPUT_PIN, "风扇开关", true);
  
  checkTimer(uvState, UV_OUTPUT_PIN, "紫外线");
  checkTimer(heaterFanState, HEATER_OUTPUT_PIN, "加热+风扇");
  checkTimer(fanOnlyState, FAN_OUTPUT_PIN, "风扇");
  
  updateTemperatureHumidity();
  
  controlOutputs();
  
  updateD5Led();
  
  updateBuzzer();
  
  printStatus();
  
  delay(5);
}
