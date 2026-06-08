#pragma once
#include "config.h"

// 创建WiFi热点
void createWiFiAP() {
  Serial.print("创建WiFi热点: ");
  Serial.println(ap_ssid);
  
  if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
    Serial.println("AP配置失败");
  }
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);
  
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP地址: ");
  Serial.println(myIP);
  Serial.println("请在手机WiFi设置中连接 '12123'，密码 '123123'");
  Serial.println("然后在浏览器中访问 http://192.168.4.1 控制设备");
  
  wifiConnected = true;
}

// 处理网页请求
void handleRoot() {
  server.send(200, "text/html", getHTMLPage());
}

// 处理控制请求
void handleControl() {
  String device = server.arg("device");
  String action = server.arg("action");
  
  Serial.print("Web控制请求: ");
  Serial.print(device);
  Serial.print(" - ");
  Serial.println(action);
  
  if (action == "toggle") {
    if (device == "uv") {
      if (uvState.functionActive) {
        uvState.functionActive = false;
        uvState.timerStart = 0;
        manualCancel = true;
        Serial.println("Web控制：停止紫外线灯");
      } else {
        uvState.functionActive = true;
        uvState.timerStart = millis();
        uvState.currentMode = globalCurrentMode;
        playBuzzerPattern(5);
        lastDeviceEnded = false;
        checkEndBeepPlayed = false;
        Serial.println("Web控制：启动紫外线灯");
      }
    }
    else if (device == "heaterfan") {
      if (heaterFanState.functionActive || autoHumidityControl) {
        heaterFanState.functionActive = false;
        heaterFanState.timerStart = 0;
        autoHumidityControl = false;
        autoHumidityLocked = true;
        manualForceStopHumidity = true;
        manualCancel = true;
        Serial.println("Web控制：强制关闭加热+风扇（包括自动湿度控制）");
      } else {
        heaterFanState.functionActive = true;
        heaterFanState.timerStart = millis();
        heaterFanState.currentMode = globalCurrentMode;
        playBuzzerPattern(5);
        lastDeviceEnded = false;
        checkEndBeepPlayed = false;
        manualForceStopTemp = false;
        manualForceStopHumidity = false;
        Serial.println("Web控制：启动加热+风扇");
      }
    }
    else if (device == "fan") {
      if (fanOnlyState.functionActive || autoTempControl || autoHumidityControl) {
        fanOnlyState.functionActive = false;
        fanOnlyState.timerStart = 0;
        autoTempControl = false;
        autoHumidityControl = false;
        autoTempLocked = true;
        autoHumidityLocked = true;
        manualForceStopTemp = true;
        manualForceStopHumidity = true;
        manualCancel = true;
        Serial.println("Web控制：强制关闭风扇（包括自动温湿度控制）");
      } else {
        fanOnlyState.functionActive = true;
        fanOnlyState.timerStart = millis();
        fanOnlyState.currentMode = globalCurrentMode;
        playBuzzerPattern(5);
        lastDeviceEnded = false;
        checkEndBeepPlayed = false;
        manualForceStopTemp = false;
        manualForceStopHumidity = false;
        Serial.println("Web控制：启动风扇单独");
      }
    }
  }
  else if (device == "mode" && action == "next") {
    TimerMode newMode = static_cast<TimerMode>((globalCurrentMode + 1) % 4);
    globalCurrentMode = newMode;
    playBuzzerPattern(newMode);
    Serial.print("Web控制：切换到");
    Serial.println(TIMER_MODE_NAMES[newMode]);
  }
  
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Redirecting to main page...");
}

// 处理404错误
void handleNotFound() {
  String message = "页面未找到\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  
  server.send(404, "text/plain", message);
}
