#pragma once
#include "config.h"

// HTML控制页面
String getHTMLPage() {
  bool uvRunning = uvState.functionActive;
  bool heaterFanRunning = heaterFanState.functionActive;
  bool fanOnlyRunning = fanOnlyState.functionActive;
  
  bool autoTempActive = autoTempControl;
  bool autoHumidityActive = autoHumidityControl;
  
  bool heaterFanTotalActive = heaterFanRunning || autoHumidityActive;
  bool fanOnlyTotalActive = fanOnlyRunning || autoTempActive || autoHumidityActive;
  
  unsigned long currentTime = millis();
  
  String uvRemaining = "已停止";
  if (uvRunning) {
    unsigned long elapsed = currentTime - uvState.timerStart;
    unsigned long duration = TIMER_DURATIONS[uvState.currentMode];
    unsigned long remaining = (duration > elapsed) ? (duration - elapsed) / 1000 : 0;
    
    if (remaining > 60) {
      uvRemaining = "剩余" + String(remaining / 60) + "分钟" + String(remaining % 60) + "秒";
    } else {
      uvRemaining = "剩余" + String(remaining) + "秒";
    }
  }
  
  String heaterFanRemaining = "已停止";
  if (heaterFanRunning) {
    unsigned long elapsed = currentTime - heaterFanState.timerStart;
    unsigned long duration = TIMER_DURATIONS[heaterFanState.currentMode];
    unsigned long remaining = (duration > elapsed) ? (duration - elapsed) / 1000 : 0;
    
    if (remaining > 60) {
      heaterFanRemaining = "剩余" + String(remaining / 60) + "分钟" + String(remaining % 60) + "秒";
    } else {
      heaterFanRemaining = "剩余" + String(remaining) + "秒";
    }
  } else if (autoHumidityActive) {
    heaterFanRemaining = "自动运行(湿度控制)";
  }
  
  String fanOnlyRemaining = "已停止";
  if (fanOnlyRunning) {
    unsigned long elapsed = currentTime - fanOnlyState.timerStart;
    unsigned long duration = TIMER_DURATIONS[fanOnlyState.currentMode];
    unsigned long remaining = (duration > elapsed) ? (duration - elapsed) / 1000 : 0;
    
    if (remaining > 60) {
      fanOnlyRemaining = "剩余" + String(remaining / 60) + "分钟" + String(remaining % 60) + "秒";
    } else {
      fanOnlyRemaining = "剩余" + String(remaining) + "秒";
    }
  } else if (autoTempActive) {
    fanOnlyRemaining = "自动运行(温度控制)";
  } else if (autoHumidityActive) {
    fanOnlyRemaining = "自动运行(湿度控制)";
  }
  
  int tempPercent = isnan(currentTemperature) ? 0 : constrain(map(currentTemperature, 0, 60, 0, 100), 0, 100);
  int humiPercent = isnan(currentHumidity) ? 0 : constrain(map(currentHumidity, 0, 100, 0, 100), 0, 100);
  
  String activeColor = "#10b981";
  String autoColor = "#3b82f6";
  String inactiveColor = "#6b7280";
  
  String html = "<!DOCTYPE html><html lang='zh-CN'>";
  html += "<head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0, user-scalable=no'>";
  html += "<title>Black Smart Controller</title>";
  html += "<meta http-equiv='refresh' content='3'>";
  html += "<style>";
  html += "* {margin: 0; padding: 0; box-sizing: border-box; font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;}";
  html += "body {background: #f8fafc; min-height: 100vh; padding: 16px;}";
  html += ".container {max-width: 400px; margin: 0 auto;}";
  html += ".card {background: white; border-radius: 12px; padding: 16px; margin-bottom: 12px; box-shadow: 0 2px 8px rgba(0,0,0,0.05);}";
  html += ".card-title {font-size: 16px; font-weight: 600; color: #1f2937; margin-bottom: 12px;}";
  html += ".sensor-card {text-align: center; padding: 20px 16px;}";
  html += ".sensor-values {font-size: 28px; font-weight: 700; color: #1f2937; margin: 12px 0;}";
  html += ".sensor-unit {font-size: 14px; color: #6b7280; margin-left: 4px;}";
  html += ".progress-bar {height: 6px; background: #e5e7eb; border-radius: 3px; margin: 6px 0; overflow: hidden;}";
  html += ".progress-fill {height: 100%; border-radius: 3px;}";
  html += ".temp-fill {background: linear-gradient(90deg, #f97316, #ef4444);}";
  html += ".humi-fill {background: linear-gradient(90deg, #3b82f6, #8b5cf6);}";
  html += ".clickable-block {display: block; text-decoration: none;}";
  html += ".device-block {padding: 16px; border-radius: 8px; background: #f9fafb; display: flex; justify-content: space-between; align-items: center; margin-bottom: 10px; transition: all 0.2s;}";
  html += ".device-block:active {background: #f3f4f6; transform: scale(0.98);}";
  html += ".device-info {display: flex; flex-direction: column;}";
  html += ".device-name {font-size: 15px; font-weight: 500; color: #1f2937; margin-bottom: 2px;}";
  html += ".device-status {font-size: 13px; color: #6b7280;}";
  html += ".device-time {font-size: 14px; font-weight: 500;}";
  html += ".timer-block {padding: 12px; border-radius: 8px; background: #eff6ff; color: #2563eb; text-align: center; font-weight: 600; margin-bottom: 12px; transition: all 0.2s;}";
  html += ".timer-block:active {background: #dbeafe; transform: scale(0.98);}";
  html += "</style>";
  html += "</head>";
  
  html += "<body>";
  html += "<div class='container'>";
  
  html += "<div class='card sensor-card'>";
  html += "<div class='card-title'>温湿度监测</div>";
  
  if (!isnan(currentTemperature) && !isnan(currentHumidity)) {
    html += "<div class='sensor-values'>";
    html += String(currentTemperature, 1) + "<span class='sensor-unit'>℃</span> / " + String(currentHumidity, 1) + "<span class='sensor-unit'>%</span>";
    html += "</div>";
    
    html += "<div style='display: flex; justify-content: space-between; font-size: 12px; color: #6b7280; margin-top: 8px;'>";
    html += "<span>温度</span>";
    html += "<span>" + String(tempPercent) + "%</span>";
    html += "</div>";
    html += "<div class='progress-bar'>";
    html += "<div class='progress-fill temp-fill' style='width: " + String(tempPercent) + "%'></div>";
    html += "</div>";
    
    html += "<div style='display: flex; justify-content: space-between; font-size: 12px; color: #6b7280; margin-top: 8px;'>";
    html += "<span>湿度</span>";
    html += "<span>" + String(humiPercent) + "%</span>";
    html += "</div>";
    html += "<div class='progress-bar'>";
    html += "<div class='progress-fill humi-fill' style='width: " + String(humiPercent) + "%'></div>";
    html += "</div>";
  } else {
    html += "<div class='sensor-values' style='color: #9ca3af;'>传感器数据读取中...</div>";
  }
  html += "</div>";
  
  html += "<div class='card'>";
  html += "<div class='card-title'>设备控制</div>";
  
  html += "<a href='/control?device=mode&action=next' class='clickable-block'>";
  html += "<div class='timer-block'>当前定时: " + String(TIMER_MODE_NAMES[globalCurrentMode]) + "</div>";
  html += "</a>";
  
  html += "<a href='/control?device=uv&action=toggle' class='clickable-block'>";
  html += "<div class='device-block'>";
  html += "<div class='device-info'>";
  html += "<div class='device-name'>紫外线灯</div>";
  html += "<div class='device-status'>" + String(uvRunning ? "手动运行" : "已停止") + "</div>";
  html += "</div>";
  html += "<div class='device-time' style='color: " + String(uvRunning ? activeColor : inactiveColor) + ";'>" + uvRemaining + "</div>";
  html += "</div>";
  html += "</a>";
  
  html += "<a href='/control?device=heaterfan&action=toggle' class='clickable-block'>";
  html += "<div class='device-block'>";
  html += "<div class='device-info'>";
  html += "<div class='device-name'>加热+风扇</div>";
  html += "<div class='device-status'>" + String(heaterFanRunning ? "手动运行" : (autoHumidityActive ? "自动运行" : "已停止")) + "</div>";
  html += "</div>";
  html += "<div class='device-time' style='color: " + String(heaterFanTotalActive ? (heaterFanRunning ? activeColor : autoColor) : inactiveColor) + ";'>" + heaterFanRemaining + "</div>";
  html += "</div>";
  html += "</a>";
  
  html += "<a href='/control?device=fan&action=toggle' class='clickable-block'>";
  html += "<div class='device-block'>";
  html += "<div class='device-info'>";
  html += "<div class='device-name'>风扇单独</div>";
  html += "<div class='device-status'>" + String(fanOnlyRunning ? "手动运行" : (autoTempActive ? "自动运行(温度)" : (autoHumidityActive ? "自动运行(湿度)" : "已停止"))) + "</div>";
  html += "</div>";
  html += "<div class='device-time' style='color: " + String(fanOnlyTotalActive ? (fanOnlyRunning ? activeColor : autoColor) : inactiveColor) + ";'>" + fanOnlyRemaining + "</div>";
  html += "</div>";
  html += "</a>";
  
  html += "</div>";
  
  html += "<div style='text-align: center; color: #6b7280; font-size: 12px; margin-top: 16px;'>";
  html += "esp32智能鞋柜 | 点击摄备块切换状态";
  html += "</div>";
  
  html += "</div>";
  html += "</body></html>";
  
  return html;
}
