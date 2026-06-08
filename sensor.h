#pragma once
#include "config.h"

// 读取DHT传感器数据
bool readDhtSensorRobust(float &temperature, float &humidity) {
  for (int i = 0; i < 3; i++) {
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    
    if (!isnan(temperature) && !isnan(humidity)) {
      if (humidity >= 0 && humidity <= 100 && temperature >= -40 && temperature <= 80) {
        return true;
      }
    }
    
    if (i < 2) {
      delay(100);
    }
  }
  
  temperature = NAN;
  humidity = NAN;
  return false;
}

// 更新温湿度数据
void updateTemperatureHumidity() {
  unsigned long currentTime = millis();
  
  if (sensorErrorCount > maxSensorErrors) {
    if (currentTime - lastSensorReadTime > 30000) {
      sensorErrorCount = 0;
      Serial.println("🔄 重新尝试读取传感器");
    }
    return;
  }
  
  if (currentTime - lastSensorReadTime > sensorReadInterval) {
    float temp, hum;
    bool readSuccess = readDhtSensorRobust(temp, hum);
    lastSensorReadTime = currentTime;
    
    if (readSuccess) {
      currentTemperature = temp;
      currentHumidity = hum;
      
      bool manualFanControl = (heaterFanState.functionActive || fanOnlyState.functionActive);
      bool manualHeaterControl = heaterFanState.functionActive;
      
      // 温度自动控制
      if (!manualFanControl) {
        if (autoTempLocked) {
          if (temp < TEMP_LOW_THRESHOLD) {
            autoTempLocked = false;
            manualForceStopTemp = false;
            Serial.print("🔓 温度 ");
            Serial.print(temp);
            Serial.print("℃ 低于 ");
            Serial.print(TEMP_LOW_THRESHOLD);
            Serial.println("℃，温度自动控制已解锁");
          } else {
            Serial.print("🔒 温度自动控制锁定中 - 温度 ");
            Serial.print(temp);
            Serial.print("℃，需要<");
            Serial.print(TEMP_LOW_THRESHOLD);
            Serial.println("℃才能解锁");
          }
        }
        
        if (!autoTempLocked && !manualForceStopTemp) {
          if (temp > TEMP_HIGH_THRESHOLD) {
            if (!autoTempControl) {
              autoTempControl = true;
              Serial.print("🌡️ 温度 ");
              Serial.print(temp);
              Serial.print("℃ 高于 ");
              Serial.print(TEMP_HIGH_THRESHOLD);
              Serial.println("℃，自动开启风扇");
              lastAutoTempControl = true;
              playBuzzerPattern(5);
            }
          } else if (temp < TEMP_LOW_THRESHOLD) {
            if (autoTempControl) {
              autoTempControl = false;
              Serial.print("❄️ 温度 ");
              Serial.print(temp);
              Serial.print("℃ 低于 ");
              Serial.print(TEMP_LOW_THRESHOLD);
              Serial.println("℃，自动关闭风扇");
              lastAutoTempControl = false;
              // 延迟到 utils 中检查
            }
          } else {
            if (autoTempControl) {
              Serial.print("📊 温度 ");
              Serial.print(temp);
              Serial.println("℃ 在正常范围内，风扇保持开启");
            }
          }
        } else if (manualForceStopTemp) {
          autoTempControl = false;
          Serial.println("🛑 温度自动控制被手动强制关闭，忽略温度条件");
        }
      }
      
      // 湿度自动控制
      if (!manualFanControl && !manualHeaterControl) {
        if (autoHumidityLocked) {
          if (hum < HUMIDITY_LOW_THRESHOLD) {
            autoHumidityLocked = false;
            manualForceStopHumidity = false;
            Serial.print("🔓 湿度 ");
            Serial.print(hum);
            Serial.print("% 低于 ");
            Serial.print(HUMIDITY_LOW_THRESHOLD);
            Serial.println("%，湿度自动控制已解锁");
          } else {
            Serial.print("🔒 湿度自动控制锁定中 - 湿度 ");
            Serial.print(hum);
            Serial.print("%，需要<");
            Serial.print(HUMIDITY_LOW_THRESHOLD);
            Serial.println("%才能解锁");
          }
        }
        
        if (!autoHumidityLocked && !manualForceStopHumidity) {
          if (hum > HUMIDITY_HIGH_THRESHOLD) {
            if (!autoHumidityControl) {
              autoHumidityControl = true;
              Serial.print("💧 湿度 ");
              Serial.print(hum);
              Serial.print("% 高于 ");
              Serial.print(HUMIDITY_HIGH_THRESHOLD);
              Serial.println("%，自动开启加热+风扇");
              lastAutoHumidityControl = true;
              playBuzzerPattern(5);
            }
          } else if (hum < HUMIDITY_LOW_THRESHOLD) {
            if (autoHumidityControl) {
              autoHumidityControl = false;
              Serial.print("🌤️ 湿度 ");
              Serial.print(hum);
              Serial.print("% 低于 ");
              Serial.print(HUMIDITY_LOW_THRESHOLD);
              Serial.println("%，自动关闭加热+风扇");
              lastAutoHumidityControl = false;
            }
          } else {
            if (autoHumidityControl) {
              Serial.print("📊 湿度 ");
              Serial.print(hum);
              Serial.println("% 在正常范围内，加热+风扇保持开启");
            }
          }
        } else if (manualForceStopHumidity) {
          autoHumidityControl = false;
          Serial.println("🛑 湿度自动控制被手动强制关闭，忽略湿度条件");
        }
      }
      
      sensorErrorCount = 0;
    } else {
      sensorErrorCount++;
      if (sensorErrorCount <= maxSensorErrors) {
        Serial.print("⚠️ 传感器读取失败 (");
        Serial.print(sensorErrorCount);
        Serial.print("/");
        Serial.print(maxSensorErrors);
        Serial.println(")");
      } else {
        Serial.println("❌ 传感器连续错误，暂停读取");
      }
    }
  }
}
