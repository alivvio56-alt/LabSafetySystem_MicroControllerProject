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

// ============================================================
// PIN CONFIGURATION
// ============================================================
#define PIN_SENSOR      A0
#define PIN_BUZZER      9
#define PIN_LED_RED     13
#define PIN_LED_GREEN   12
#define PIN_BUTTON      2

// ============================================================
// LCD CONFIGURATION
// ============================================================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ============================================================
// GLOBAL SHARED VARIABLES
// ============================================================
volatile int  thresholdValue = 100;
volatile int  sensorValue    = 0;

volatile bool systemLocked   = false;
volatile bool alarmMuted     = false;

// ============================================================
// SEMAPHORE HANDLE
// ============================================================
SemaphoreHandle_t muteSemaphore;

// ============================================================
// CRITICAL SECTION MACRO
// ============================================================
#define ENTER_CRITICAL() taskENTER_CRITICAL()
#define EXIT_CRITICAL()  taskEXIT_CRITICAL()

// ============================================================
// INTERRUPT SERVICE ROUTINE
// ============================================================
void ISR_MuteButton() {

  alarmMuted = true;

  BaseType_t higherPriorityTaskWoken = pdFALSE;

  xSemaphoreGiveFromISR(
    muteSemaphore,
    &higherPriorityTaskWoken
  );

  portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

// ============================================================
// TASK 1 — SENSOR READING
// ============================================================
void vTaskSensor(void *pvParameters) {

  (void) pvParameters;

  while (1) {

    int adc = analogRead(PIN_SENSOR);

    ENTER_CRITICAL();
    sensorValue = adc;
    EXIT_CRITICAL();

    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

// ============================================================
// TASK 2 — ALARM CONTROL
// ============================================================
void vTaskAlarm(void *pvParameters) {

  (void) pvParameters;

  while (1) {

    ENTER_CRITICAL();

    int  adc    = sensorValue;
    int  limit  = thresholdValue;
    bool locked = systemLocked;
    bool muted  = alarmMuted;

    EXIT_CRITICAL();

    // ========================================================
    // SYSTEM LOCKED
    // ========================================================
    if (locked) {

      noTone(PIN_BUZZER);

      digitalWrite(PIN_LED_RED, LOW);
      digitalWrite(PIN_LED_GREEN, LOW);

      vTaskDelay(pdMS_TO_TICKS(200));
      continue;
    }

    // ========================================================
    // DANGER CONDITION
    // ========================================================
    if (adc > limit) {

      digitalWrite(PIN_LED_RED, HIGH);
      digitalWrite(PIN_LED_GREEN, LOW);

      if (!muted) {

        // Sweep up
        for (int freq = 600; freq <= 1200; freq += 20) {

          if (alarmMuted || systemLocked) {
            break;
          }

          tone(PIN_BUZZER, freq);

          vTaskDelay(pdMS_TO_TICKS(8));
        }

        // Sweep down
        for (int freq = 1200; freq >= 600; freq -= 20) {

          if (alarmMuted || systemLocked) {
            break;
          }

          tone(PIN_BUZZER, freq);

          vTaskDelay(pdMS_TO_TICKS(8));
        }

      } else {

        noTone(PIN_BUZZER);

        vTaskDelay(pdMS_TO_TICKS(300));
      }

    }

    // ========================================================
    // SAFE CONDITION
    // ========================================================
    else {

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

// ============================================================
// TASK 3 — LCD DISPLAY
// ============================================================
void vTaskLCD(void *pvParameters) {

  (void) pvParameters;

  while (1) {

    ENTER_CRITICAL();

    int  adc    = sensorValue;
    int  limit  = thresholdValue;
    bool locked = systemLocked;
    bool muted  = alarmMuted;

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

// ============================================================
// TASK 4 — SERIAL MONITOR
// COMMAND:
//   on
//   lock
//   set [0-1023]
// ============================================================
void vTaskSerial(void *pvParameters) {

  (void) pvParameters;

  while (1) {

    // ========================================================
    // READ SERIAL COMMAND
    // ========================================================
    if (Serial.available()) {

      String cmd = Serial.readStringUntil('\n');

      cmd.trim();
      cmd.toLowerCase();

      // ------------------------------------------------------
      // LOCK SYSTEM
      // ------------------------------------------------------
      if (cmd == "lock") {

        ENTER_CRITICAL();
        systemLocked = true;
        EXIT_CRITICAL();

        noTone(PIN_BUZZER);

        Serial.println("[UART] Sistem dikunci");

      }

      // ------------------------------------------------------
      // TURN SYSTEM ON
      // ------------------------------------------------------
      else if (cmd == "on") {

        ENTER_CRITICAL();

        systemLocked = false;
        alarmMuted   = false;

        EXIT_CRITICAL();

        Serial.println("[UART] Sistem aktif");
      }

      // ------------------------------------------------------
      // SET THRESHOLD
      // ------------------------------------------------------
      else if (cmd.startsWith("set ")) {

        int value = cmd.substring(4).toInt();

        if (value >= 0 && value <= 1023) {

          ENTER_CRITICAL();
          thresholdValue = value;
          EXIT_CRITICAL();

          Serial.print("[SET] Threshold = ");
          Serial.println(value);

        } else {

          Serial.println("[ERROR] Range 0-1023");
        }
      }

      // ------------------------------------------------------
      // UNKNOWN COMMAND
      // ------------------------------------------------------
      else {

        Serial.print("[ERROR] Unknown command: ");
        Serial.println(cmd);
      }
    }

    // ========================================================
    // ADC LOGGING
    // ========================================================
    ENTER_CRITICAL();
    int adc = sensorValue;
    EXIT_CRITICAL();

    Serial.print("[ADC] ");
    Serial.println(adc);

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

// ============================================================
// SETUP
// ============================================================
void setup() {

  // ==========================================================
  // SERIAL
  // ==========================================================
  Serial.begin(9600);

  Serial.println("================================");
  Serial.println(" Lab Safety System - FreeRTOS ");
  Serial.println("================================");

  Serial.println("Command:");
  Serial.println("on");
  Serial.println("lock");
  Serial.println("set [0-1023]");

  // ==========================================================
  // PIN MODE
  // ==========================================================
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  pinMode(PIN_BUTTON, INPUT_PULLUP);

  // ==========================================================
  // INTERRUPT
  // ==========================================================
  attachInterrupt(
    digitalPinToInterrupt(PIN_BUTTON),
    ISR_MuteButton,
    FALLING
  );

  // ==========================================================
  // LCD INIT
  // ==========================================================
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Lab Safety Syst");

  lcd.setCursor(0, 1);
  lcd.print("Initializing...");

  delay(1500);

  lcd.clear();

  // ==========================================================
  // CREATE SEMAPHORE
  // ==========================================================
  muteSemaphore = xSemaphoreCreateBinary();

  if (muteSemaphore == NULL) {

    Serial.println("[ERROR] Semaphore gagal dibuat");

    while (1);
  }

  // ==========================================================
  // CREATE TASKS
  // ==========================================================
  xTaskCreate(
    vTaskSensor,
    "Sensor",
    128,
    NULL,
    2,
    NULL
  );

  xTaskCreate(
    vTaskAlarm,
    "Alarm",
    256,
    NULL,
    3,
    NULL
  );

  xTaskCreate(
    vTaskLCD,
    "LCD",
    192,
    NULL,
    1,
    NULL
  );

  xTaskCreate(
    vTaskSerial,
    "Serial",
    256,
    NULL,
    2,
    NULL
  );

  // ==========================================================
  // START SCHEDULER
  // ==========================================================
  vTaskStartScheduler();

  // ==========================================================
  // ERROR IF FAILED
  // ==========================================================
  Serial.println("[ERROR] Scheduler gagal");

  while (1);
}

// ============================================================
// LOOP NOT USED
// ============================================================
void loop() {
}
