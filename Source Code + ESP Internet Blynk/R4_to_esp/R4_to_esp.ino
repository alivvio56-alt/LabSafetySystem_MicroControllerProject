/*
 * ============================================================
 *  Lab Safety System — FreeRTOS Edition
 *  Board  : Arduino UNO R4 Minima
 *  RTOS   : FreeRTOS Built-in
 * ============================================================
 */

#include <Arduino_FreeRTOS.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define PIN_SENSOR      A0
#define PIN_BUZZER      9
#define PIN_LED_RED     13
#define PIN_LED_GREEN   12
#define PIN_BUTTON      2

LiquidCrystal_I2C lcd(0x27, 16, 2);

volatile int thresholdValue = 350;
volatile int sensorValue = 0;

volatile bool systemLocked = false;
volatile bool alarmMuted = false;

SemaphoreHandle_t muteSemaphore;

#define ENTER_CRITICAL() taskENTER_CRITICAL()
#define EXIT_CRITICAL() taskEXIT_CRITICAL()

void ISR_MuteButton() {

  alarmMuted = true;

  BaseType_t higherPriorityTaskWoken = pdFALSE;

  xSemaphoreGiveFromISR(
    muteSemaphore,
    &higherPriorityTaskWoken
  );

  portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

void vTaskSensor(void *pvParameters) {

  while (1) {

    int adc = analogRead(PIN_SENSOR);

    ENTER_CRITICAL();
    sensorValue = adc;
    EXIT_CRITICAL();

    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

void vTaskAlarm(void *pvParameters) {

  while (1) {

    ENTER_CRITICAL();

    int adc = sensorValue;
    int limit = thresholdValue;
    bool locked = systemLocked;
    bool muted = alarmMuted;

    EXIT_CRITICAL();

    if (locked) {

      noTone(PIN_BUZZER);

      digitalWrite(PIN_LED_RED, LOW);
      digitalWrite(PIN_LED_GREEN, LOW);

      vTaskDelay(pdMS_TO_TICKS(200));
      continue;
    }

    if (adc > limit) {

      digitalWrite(PIN_LED_RED, HIGH);
      digitalWrite(PIN_LED_GREEN, LOW);

      if (!muted) {

        for (int freq = 600; freq <= 1200; freq += 20) {

          if (alarmMuted || systemLocked)
            break;

          tone(PIN_BUZZER, freq);

          vTaskDelay(pdMS_TO_TICKS(8));
        }

        for (int freq = 1200; freq >= 600; freq -= 20) {

          if (alarmMuted || systemLocked)
            break;

          tone(PIN_BUZZER, freq);

          vTaskDelay(pdMS_TO_TICKS(8));
        }

      } else {

        noTone(PIN_BUZZER);

        vTaskDelay(pdMS_TO_TICKS(300));
      }

    } else {

      noTone(PIN_BUZZER);

      digitalWrite(PIN_LED_RED, LOW);
      digitalWrite(PIN_LED_GREEN, HIGH);

      ENTER_CRITICAL();
      alarmMuted = false;
      EXIT_CRITICAL();

      vTaskDelay(pdMS_TO_TICKS(300));
    }
  }
}

void vTaskLCD(void *pvParameters) {

  while (1) {

    ENTER_CRITICAL();

    int adc = sensorValue;
    int limit = thresholdValue;
    bool locked = systemLocked;
    bool muted = alarmMuted;

    EXIT_CRITICAL();

    lcd.setCursor(0, 0);
    lcd.print("Lab Safety Syst");

    lcd.setCursor(0, 1);

    if (locked) {

      lcd.print("System LOCKED ");

    } else if (adc > limit && muted) {

      lcd.print("Muted Val:");
      lcd.print(adc);
      lcd.print("   ");

    } else if (adc > limit) {

      lcd.print("BAHAYA:");
      lcd.print(adc);
      lcd.print("      ");

    } else {

      lcd.print("AMAN:");
      lcd.print(adc);
      lcd.print("        ");
    }

    vTaskDelay(pdMS_TO_TICKS(400));
  }
}

void vTaskSerial(void *pvParameters) {

  while (1) {

    // ============================================
    // COMMAND DARI USB SERIAL
    // ============================================
    if (Serial.available()) {

      String cmd =
        Serial.readStringUntil('\n');

      cmd.trim();
      cmd.toLowerCase();

      if (cmd == "lock") {

        ENTER_CRITICAL();
        systemLocked = true;
        EXIT_CRITICAL();

        noTone(PIN_BUZZER);

      }

      else if (cmd == "on") {

        ENTER_CRITICAL();

        systemLocked = false;
        alarmMuted = false;

        EXIT_CRITICAL();
      }

      else if (cmd.startsWith("set ")) {

        int value =
          cmd.substring(4).toInt();

        if (value >= 0 &&
            value <= 1023) {

          ENTER_CRITICAL();

          thresholdValue = value;

          EXIT_CRITICAL();
        }
      }
    }

    // ============================================
    // COMMAND DARI ESP32
    // ============================================
    if (Serial1.available()) {

      String cmd =
        Serial1.readStringUntil('\n');

      cmd.trim();
      cmd.toUpperCase();

      if (cmd == "LOCK") {

        ENTER_CRITICAL();

        systemLocked = true;

        EXIT_CRITICAL();

        noTone(PIN_BUZZER);

        Serial.println("[ESP] LOCK");
      }

      else if (cmd == "RESET") {

        ENTER_CRITICAL();

        systemLocked = false;
        alarmMuted = false;

        EXIT_CRITICAL();

        Serial.println("[ESP] RESET");
      }

      else if (cmd == "MUTE") {

        ENTER_CRITICAL();

        alarmMuted = true;

        EXIT_CRITICAL();

        Serial.println("[ESP] MUTE");
      }

      else if (cmd.startsWith("THRESH:")) {

        int value =
          cmd.substring(7).toInt();

        if (value >= 0 &&
            value <= 1023) {

          ENTER_CRITICAL();

          thresholdValue = value;

          EXIT_CRITICAL();

          Serial.print("[ESP] THRESH=");
          Serial.println(value);
        }
      }
    }

    // ============================================
    // SEND DATA TO ESP32
    // ============================================

    ENTER_CRITICAL();

    int adc = sensorValue;
    int limit = thresholdValue;
    bool locked = systemLocked;
    bool muted = alarmMuted;

    EXIT_CRITICAL();

    Serial1.print("ADC:");
    Serial1.print(adc);

    Serial1.print(",LIMIT:");
    Serial1.print(limit);

    Serial1.print(",LOCK:");
    Serial1.print(locked);

    Serial1.print(",MUTE:");
    Serial1.println(muted);

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void setup() {

  Serial.begin(9600);

  // UART KE ESP32
  Serial1.begin(9600);

  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  attachInterrupt(
    digitalPinToInterrupt(PIN_BUTTON),
    ISR_MuteButton,
    FALLING
  );

  lcd.init();
  lcd.backlight();

  muteSemaphore = xSemaphoreCreateBinary();

  xTaskCreate(vTaskSensor, "Sensor", 128, NULL, 2, NULL);
  xTaskCreate(vTaskAlarm,  "Alarm", 256, NULL, 3, NULL);
  xTaskCreate(vTaskLCD,    "LCD",   192, NULL, 1, NULL);
  xTaskCreate(vTaskSerial, "Serial",256, NULL, 2, NULL);

  vTaskStartScheduler();

  while (1);
}

void loop() {}
