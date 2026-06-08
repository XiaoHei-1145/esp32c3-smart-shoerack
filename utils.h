#pragma once
#include "config.h"

// 检查是否有任何设备在运行
bool isAnyDeviceRunning() {
  return (uvState.functionActive || 
          heaterFanState.functionActive || 
          fanOnlyState.functionActive ||
          autoTempControl ||
          autoHumidityControl);
}

// 检查是否有任何手动设备在运行
bool isAnyManualDeviceRunning() {
  return (uvState.functionActive || 
          heaterFanState.functionActive || 
          fanOnlyState.functionActive);
}

// 检查是否有任何自动设备在运行
bool isAnyAutoDeviceRunning() {
  return (autoTempControl || autoHumidityControl);
}

// 更新D5 LED状态
void updateD5Led() {
  digitalWrite(D5_LED_PIN, isAnyDeviceRunning() ? HIGH : LOW);
}

// 检查并播放结束音（用于自动控制关闭）
void checkAutoEndBeep() {
  if (!isAnyDeviceRunning() && !checkEndBeepPlayed) {
    playBuzzerPattern(4);
    checkEndBeepPlayed = true;
    lastDeviceEnded = true;
    Serial.println("📢 自动控制设备全部关闭，播放结束提示");
  }
}

// 获取设备状态图标
String getDeviceStatusIcon(bool isRunning) {
  return isRunning ? "🟢" : "🔴";
}

// 打印状态信息
void printStatus() {
  static unsigned long lastPrintTime = 0;
  unsigned long currentTime = millis();
  
  if (currentTime - lastPrintTime < 3000) return;
  
  lastPrintTime = currentTime;
  
  String deviceStatus = "";
  
  bool fanRunning = (heaterFanState.functionActive || 
                    fanOnlyState.functionActive || 
                    autoTempControl || 
                    autoHumidityControl);
  deviceStatus += "风扇" + getDeviceStatusIcon(fanRunning);
  
  bool heaterRunning = (heaterFanState.functionActive || autoHumidityControl);
  deviceStatus += " 加热丝" + getDeviceStatusIcon(heaterRunning);
  
  bool uvRunning = uvState.functionActive;
  deviceStatus += " 紫外线" + getDeviceStatusIcon(uvRunning);
  
  String activeFunctions = "";
  
  if (uvState.functionActive) {
    unsigned long elapsed = currentTime - uvState.timerStart;
    unsigned long duration = TIMER_DURATIONS[uvState.currentMode];
    unsigned long remaining = (duration > elapsed) ? (duration - elapsed) / 1000 : 0;
    
    if (activeFunctions.length() > 0) activeFunctions += " | ";
    activeFunctions += "紫外线(" + String(remaining) + "s, 挡位:" + TIMER_MODE_NAMES[uvState.currentMode] + ")";
  }
  
  if (heaterFanState.functionActive) {
    unsigned long elapsed = currentTime - heaterFanState.timerStart;
    unsigned long duration = TIMER_DURATIONS[heaterFanState.currentMode];
    unsigned long remaining = (duration > elapsed) ? (duration - elapsed) / 1000 : 0;
    
    if (activeFunctions.length() > 0) activeFunctions += " | ";
    activeFunctions += "加热+风扇(" + String(remaining) + "s, 挡位:" + TIMER_MODE_NAMES[heaterFanState.currentMode] + ")";
  }
  
  if (fanOnlyState.functionActive) {
    unsigned long elapsed = currentTime - fanOnlyState.timerStart;
    unsigned long duration = TIMER_DURATIONS[fanOnlyState.currentMode];
    unsigned long remaining = (duration > elapsed) ? (duration - elapsed) / 1000 : 0;
    
    if (activeFunctions.length() > 0) activeFunctions += " | ";
    activeFunctions += "风扇(" + String(remaining) + "s, 挡位:" + TIMER_MODE_NAMES[fanOnlyState.currentMode] + ")";
  }
  
  if (autoTempControl) {
    if (activeFunctions.length() > 0) activeFunctions += " | ";
    activeFunctions += "自动温度控制";
  }
  
  if (autoHumidityControl) {
    if (activeFunctions.length() > 0) activeFunctions += " | ";
    activeFunctions += "自动湿度控制";
  }
  
  String waitingStates = "";
  if (uvState.waitingForRelease) waitingStates += "紫外线";
  if (heaterFanState.waitingForRelease) {
    if (waitingStates.length() > 0) waitingStates += ",";
    waitingStates += "加热+风扇";
  }
  if (fanOnlyState.waitingForRelease) {
    if (waitingStates.length() > 0) waitingStates += ",";
    waitingStates += "风扇";
  }
  
  String statusMsg = deviceStatus;
  
  if (activeFunctions.length() > 0) {
    statusMsg += " | " + activeFunctions;
  } else {
    statusMsg += " | 系统待机";
  }
  
  if (waitingStates.length() > 0) {
    statusMsg += " | 等待释放:" + waitingStates;
  }
  
  if (!isnan(currentTemperature) && !isnan(currentHumidity)) {
    statusMsg += " | " + String(currentTemperature, 1) + "℃ " + String(currentHumidity, 1) + "%";
  } else {
    if (sensorErrorCount > maxSensorErrors) {
      statusMsg += " | 传感器暂停";
    } else {
      statusMsg += " | 传感器读取中";
    }
  }
  
  String lockStatus = "";
  if (autoTempLocked) lockStatus += "温度锁定";
  if (autoHumidityLocked) {
    if (lockStatus.length() > 0) lockStatus += ",";
    lockStatus += "湿度锁定";
  }
  
  if (manualForceStopTemp) {
    if (lockStatus.length() > 0) lockStatus += ",";
    lockStatus += "温度强制关闭";
  }
  if (manualForceStopHumidity) {
    if (lockStatus.length() > 0) lockStatus += ",";
    lockStatus += "湿度强制关闭";
  }
  
  if (lockStatus.length() > 0) {
    statusMsg += " | 🔒" + lockStatus;
  }
  
  if (isAnyDeviceRunning()) {
    statusMsg += " | 💡D5灯亮";
  }
  
  statusMsg += " | ⏰当前全局挡位:";
  statusMsg += TIMER_MODE_NAMES[globalCurrentMode];
  
  String buzzerStatus = "";
  switch (buzzerState) {
    case BUZZER_IDLE: buzzerStatus = "空闲"; break;
    case BUZZER_PLAYING: buzzerStatus = "播放模式"; break;
    case BUZZER_START_DELAY: buzzerStatus = "启动延迟"; break;
    case BUZZER_START_BEEPING: buzzerStatus = "启动响铃"; break;
  }
  statusMsg += " | 🔊" + buzzerStatus;
  
  if (lastDeviceEnded) {
    statusMsg += " | 📢已播放结束提示";
  }
  
  statusMsg += " | 📶WiFi:AP模式(";
  statusMsg += ap_ssid;
  statusMsg += ")";
  
  Serial.println(statusMsg);
}
