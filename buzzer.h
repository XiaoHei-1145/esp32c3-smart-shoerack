#pragma once
#include "config.h"

// 播放蜂鸣器模式
void playBuzzerPattern(int patternIndex) {
  // 启动音效优先级最高，打断正在播放的声音
  if (patternIndex == 5) {
    buzzerState = BUZZER_START_DELAY;
    buzzerTimer = millis();
    Serial.println("📢 设备启动，准备播放启动音效 (延迟0.2秒)");
  } else {
    buzzerState = BUZZER_PLAYING;
    buzzerTimer = millis();
    currentBuzzerPattern = patternIndex;
    buzzerStepIndex = 0;
    
    if (patternIndex == 4) {
      Serial.print("📢 最后一个设备定时结束，播放结束提示: ");
      Serial.println(BUZZER_PATTERNS[patternIndex]);
    } else {
      Serial.print("📢 蜂鸣器播放挡位");
      Serial.print(TIMER_MODE_NAMES[patternIndex]);
      Serial.print("模式: ");
      Serial.println(BUZZER_PATTERNS[patternIndex]);
    }
  }
}

// 更新蜂鸣器状态
void updateBuzzer() {
  unsigned long currentTime = millis();
  
  switch (buzzerState) {
    case BUZZER_IDLE:
      break;
      
    case BUZZER_PLAYING: {
      const char* pattern = BUZZER_PATTERNS[currentBuzzerPattern];
      int patternLength = strlen(pattern);
      
      if (buzzerStepIndex >= patternLength) {
        buzzerState = BUZZER_IDLE;
        digitalWrite(BUZZER_PIN, LOW);
        break;
      }
      
      unsigned long elapsed = currentTime - buzzerTimer;
      char currentChar = pattern[buzzerStepIndex];
      
      if (currentChar == '1' || currentChar == '0') {
        unsigned long signalDuration = (currentChar == '1') ? 600 : 100;
        
        if (elapsed < signalDuration) {
          digitalWrite(BUZZER_PIN, HIGH);
        } else if (elapsed < signalDuration + 400) {
          digitalWrite(BUZZER_PIN, LOW);
        } else {
          buzzerStepIndex++;
          buzzerTimer = currentTime;
        }
      }
      break;
    }
    
    case BUZZER_START_DELAY:
      if (currentTime - buzzerTimer >= 200) {
        buzzerState = BUZZER_START_BEEPING;
        buzzerTimer = currentTime;
        digitalWrite(BUZZER_PIN, HIGH);
        Serial.println("📢 启动音效开始 (响0.1秒)");
      }
      break;
      
    case BUZZER_START_BEEPING:
      if (currentTime - buzzerTimer >= 100) {
        buzzerState = BUZZER_IDLE;
        digitalWrite(BUZZER_PIN, LOW);
        Serial.println("📢 启动音效结束");
      }
      break;
  }
}
