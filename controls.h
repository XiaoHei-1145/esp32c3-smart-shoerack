#pragma once
#include "config.h"

// 检查定时调节按钮
void checkTimerSwitch() {
  int currentState = digitalRead(TIMER_SWITCH_PIN);
  
  if (currentState != lastTimerSwitchState) {
    lastTimerSwitchTime = millis();
  }
  
  if (millis() - lastTimerSwitchTime > 20) {
    if (currentState != timerSwitchState) {
      timerSwitchState = currentState;
      
      if (timerSwitchState == LOW) {
        if (!timerSwitchWaitingForRelease) {
          timerSwitchPressTime = millis();
          Serial.println("定时调节按钮按下");
        }
      }
      else {
        if (timerSwitchWaitingForRelease) {
          timerSwitchWaitingForRelease = false;
          Serial.println("定时调节按钮释放，等待结束");
          lastTimerSwitchState = currentState;
          return;
        }
        
        unsigned long pressDuration = millis() - timerSwitchPressTime;
        Serial.print("定时调节按钮释放，按下时长: ");
        Serial.print(pressDuration);
        Serial.println("ms");
        
        if (pressDuration > SHORT_PRESS_MIN && pressDuration < SHORT_PRESS_MAX) {
          TimerMode newMode = static_cast<TimerMode>((globalCurrentMode + 1) % 4);
          globalCurrentMode = newMode;
          
          Serial.print("⏰ 切换到");
          Serial.println(TIMER_MODE_NAMES[newMode]);
          
          playBuzzerPattern(newMode);
        }
      }
    }
  }
  
  if (timerSwitchState == LOW && !timerSwitchWaitingForRelease) {
    unsigned long pressDuration = millis() - timerSwitchPressTime;
    
    if (pressDuration > ULTRA_LONG_PRESS) {
      timerSwitchWaitingForRelease = true;
      Serial.println("定时调节按钮超长按，等待释放...");
    }
  }
  
  lastTimerSwitchState = currentState;
}

// 检查功能开关
void checkSwitch(int switchPin, FunctionState &state, int outputPin, const char* functionName, bool isFanRelated = false) {
  int currentState = digitalRead(switchPin);
  
  if (currentState != state.lastBtnState) {
    state.lastBtnTime = millis();
  }
  
  if (millis() - state.lastBtnTime > 20) {
    if (currentState != state.btnState) {
      state.btnState = currentState;
      
      if (state.btnState == LOW) {
        if (!state.waitingForRelease) {
          state.btnPressTime = millis();
          Serial.print(functionName);
          Serial.println("按下");
        }
      }
      else {
        if (state.waitingForRelease) {
          state.waitingForRelease = false;
          Serial.print(functionName);
          Serial.println("释放，等待结束");
          state.lastBtnState = currentState;
          return;
        }
        
        unsigned long pressDuration = millis() - state.btnPressTime;
        Serial.print(functionName);
        Serial.print("释放，按下时长: ");
        Serial.print(pressDuration);
        Serial.println("ms");
        
        if (pressDuration > SHORT_PRESS_MIN && pressDuration < SHORT_PRESS_MAX) {
          state.functionActive = true;
          state.timerStart = millis();
          state.currentMode = globalCurrentMode;
          
          Serial.print("⚡ 短按 - ");
          Serial.print(functionName);
          Serial.print("启动 (");
          Serial.print(TIMER_MODE_NAMES[state.currentMode]);
          Serial.println("定时)");
          
          playBuzzerPattern(5);
          
          if (isAnyManualDeviceRunning()) {
            lastDeviceEnded = false;
            checkEndBeepPlayed = false;
          }
          
          if (isFanRelated) {
            manualForceStopTemp = false;
            manualForceStopHumidity = false;
            Serial.println("🔓 取消手动强制关闭自动控制");
          }
        }
        else if (pressDuration >= LONG_PRESS_MIN && pressDuration <= LONG_PRESS_MAX) {
          if (state.functionActive) {
            state.functionActive = false;
            state.timerStart = 0;
            manualCancel = true;
            
            Serial.print("🔄 长按 - ");
            Serial.print(functionName);
            Serial.println("手动取消");
            
            if (!isAnyManualDeviceRunning() && !checkEndBeepPlayed && !manualCancel) {
              playBuzzerPattern(4);
              checkEndBeepPlayed = true;
              lastDeviceEnded = true;
            }
          }
          else if (isFanRelated) {
            if (autoTempControl && !autoTempLocked) {
              autoTempControl = false;
              autoTempLocked = true;
              manualForceStopTemp = true;
              Serial.print("🔒 长按 - 强制关闭自动温度控制并锁定");
              Serial.print("，需要温度<");
              Serial.print(TEMP_LOW_THRESHOLD);
              Serial.println("℃才能重新启用");
            }
            if (autoHumidityControl && !autoHumidityLocked) {
              autoHumidityControl = false;
              autoHumidityLocked = true;
              manualForceStopHumidity = true;
              Serial.print("🔒 长按 - 强制关闭自动湿度控制并锁定");
              Serial.print("，需要湿度<");
              Serial.print(HUMIDITY_LOW_THRESHOLD);
              Serial.println("%才能重新启用");
            }
          }
        }
      }
    }
  }
  
  if (state.btnState == LOW && !state.waitingForRelease) {
    unsigned long pressDuration = millis() - state.btnPressTime;
    
    if (pressDuration > ULTRA_LONG_PRESS) {
      if (state.functionActive) {
        state.functionActive = false;
        state.timerStart = 0;
        state.waitingForRelease = true;
        manualCancel = true;
        
        Serial.print("⏰ 超长按 - ");
        Serial.print(functionName);
        Serial.println("手动取消，等待释放...");
        
        if (!isAnyManualDeviceRunning() && !checkEndBeepPlayed && !manualCancel) {
          playBuzzerPattern(4);
          checkEndBeepPlayed = true;
          lastDeviceEnded = true;
        }
      }
      else if (isFanRelated) {
        state.waitingForRelease = true;
        if (autoTempControl && !autoTempLocked) {
          autoTempControl = false;
          autoTempLocked = true;
          manualForceStopTemp = true;
          Serial.print("⏰ 超长按 - 强制关闭自动温度控制并锁定");
          Serial.print("，需要温度<");
          Serial.print(TEMP_LOW_THRESHOLD);
          Serial.println("℃才能重新启用，等待释放...");
        }
        if (autoHumidityControl && !autoHumidityLocked) {
          autoHumidityControl = false;
          autoHumidityLocked = true;
          manualForceStopHumidity = true;
          Serial.print("⏰ 超长按 - 强制关闭自动湿度控制并锁定");
          Serial.print("，需要湿度<");
          Serial.print(HUMIDITY_LOW_THRESHOLD);
          Serial.println("%才能重新启用，等待释放...");
        }
      }
    }
  }
  
  state.lastBtnState = currentState;
}

// 检查定时器
bool checkTimer(FunctionState &state, int outputPin, const char* functionName) {
  if (state.functionActive) {
    unsigned long elapsed = millis() - state.timerStart;
    if (elapsed > TIMER_DURATIONS[state.currentMode]) {
      state.functionActive = false;
      state.timerStart = 0;
      manualCancel = false;
      
      Serial.print("✅ ");
      Serial.print(functionName);
      Serial.print(" - ");
      Serial.print(TIMER_MODE_NAMES[state.currentMode]);
      Serial.println("定时结束");
      
      if (!isAnyManualDeviceRunning() && !checkEndBeepPlayed && !manualCancel) {
        playBuzzerPattern(4);
        checkEndBeepPlayed = true;
        lastDeviceEnded = true;
      }
      
      return true;
    }
  }
  return false;
}

// 控制输出
void controlOutputs() {
  digitalWrite(UV_OUTPUT_PIN, uvState.functionActive ? HIGH : LOW);
  
  if (heaterFanState.functionActive) {
    digitalWrite(HEATER_OUTPUT_PIN, HIGH);
  } else if (autoHumidityControl) {
    digitalWrite(HEATER_OUTPUT_PIN, HIGH);
  } else {
    digitalWrite(HEATER_OUTPUT_PIN, LOW);
  }
  
  if (heaterFanState.functionActive || fanOnlyState.functionActive) {
    digitalWrite(FAN_OUTPUT_PIN, HIGH);
  } else if (autoTempControl) {
    digitalWrite(FAN_OUTPUT_PIN, HIGH);
  } else if (autoHumidityControl) {
    digitalWrite(FAN_OUTPUT_PIN, HIGH);
  } else {
    digitalWrite(FAN_OUTPUT_PIN, LOW);
  }
}
