/*
 * ============================================================
 *  Lab Safety System — FreeRTOS Edition
 *  Board  : Arduino UNO R4 Minima
 *  RTOS   : FreeRTOS Built-in
 * ============================================================
 *  Sensor baru (pin tidak mengubah pin lama):
 *    DHT22  : VCC=5V, GND, OUT -> PIN 7
 *    Flame  : VCC=5V, GND, D0  -> PIN 8
 * ============================================================
 */

#include <Arduino_FreeRTOS.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// ── Pin lama  ────────────────────────────────
#define PIN_SENSOR      A0
#define PIN_BUZZER      9
#define PIN_LED_RED     13
#define PIN_LED_GREEN   12
#define PIN_BUTTON      2

// ── Pin baru ───────────────────────────────────────────────
#define PIN_DHT         7
#define PIN_FLAME       8

#define DHT_TYPE        DHT22

// ──────────────────────────────────────────────────────────
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(PIN_DHT, DHT_TYPE);

// ── Shared variables ──────────────────────────────────────
volatile int   thresholdValue  = 350;
volatile int   tempThreshold   = 40;
volatile int   flameThreshold  = 1;

volatile int   sensorValue     = 0;
volatile float tempValue       = 0.0;
volatile bool  flameDetected   = false;

volatile bool  systemLocked    = false;
volatile bool  alarmMuted      = false;

SemaphoreHandle_t muteSemaphore;

#define ENTER_CRITICAL() taskENTER_CRITICAL()
#define EXIT_CRITICAL()  taskEXIT_CRITICAL()

// ── Helper: cek kondisi bahaya gabungan ───────────────────
inline bool isDanger(int adc, int limit, float temp, int tLimit, bool flame, int fLimit) {
  return (adc > limit) || (temp > (float)tLimit) || (flame && fLimit == 1);
}

// ── Helper: tulis satu baris LCD tepat 16 karakter ────────
// Otomatis padding spasi di kanan agar tidak ada ghost char
void lcdPrintRow(int row, String text) {
  while (text.length() < 16) text += ' ';
  lcd.setCursor(0, row);
  lcd.print(text.substring(0, 16));
}

// ── ISR ───────────────────────────────────────────────────
void ISR_MuteButton() {
  alarmMuted = true;
  BaseType_t higherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(muteSemaphore, &higherPriorityTaskWoken);
  portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

// ═══════════════════════════════════════════════════════════
//  TASK: Sensor (ADC + DHT22 + Flame)
// ═══════════════════════════════════════════════════════════
void vTaskSensor(void *pvParameters) {
  while (1) {
    int   adc   = analogRead(PIN_SENSOR);
    float temp  = dht.readTemperature();
    bool  flame = (digitalRead(PIN_FLAME) == LOW);

    if (isnan(temp)) temp = 0.0;

    ENTER_CRITICAL();
    sensorValue   = adc;
    tempValue     = temp;
    flameDetected = flame;
    EXIT_CRITICAL();

    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

// ═══════════════════════════════════════════════════════════
//  TASK: Alarm
// ═══════════════════════════════════════════════════════════
void vTaskAlarm(void *pvParameters) {
  while (1) {
    ENTER_CRITICAL();
    int   adc    = sensorValue;
    int   limit  = thresholdValue;
    float temp   = tempValue;
    int   tLimit = tempThreshold;
    bool  flame  = flameDetected;
    int   fLimit = flameThreshold;
    bool  locked = systemLocked;
    bool  muted  = alarmMuted;
    EXIT_CRITICAL();

    if (locked) {
      noTone(PIN_BUZZER);
      digitalWrite(PIN_LED_RED,   LOW);
      digitalWrite(PIN_LED_GREEN, LOW);
      vTaskDelay(pdMS_TO_TICKS(200));
      continue;
    }

    bool danger = isDanger(adc, limit, temp, tLimit, flame, fLimit);

    if (danger) {
      digitalWrite(PIN_LED_RED,   HIGH);
      digitalWrite(PIN_LED_GREEN, LOW);

      if (!muted) {
        for (int freq = 600; freq <= 1200; freq += 20) {
          if (alarmMuted || systemLocked) break;
          tone(PIN_BUZZER, freq);
          vTaskDelay(pdMS_TO_TICKS(8));
        }
        for (int freq = 1200; freq >= 600; freq -= 20) {
          if (alarmMuted || systemLocked) break;
          tone(PIN_BUZZER, freq);
          vTaskDelay(pdMS_TO_TICKS(8));
        }
      } else {
        noTone(PIN_BUZZER);
        vTaskDelay(pdMS_TO_TICKS(300));
      }

    } else {
      noTone(PIN_BUZZER);
      digitalWrite(PIN_LED_RED,   LOW);
      digitalWrite(PIN_LED_GREEN, HIGH);

      ENTER_CRITICAL();
      alarmMuted = false;
      EXIT_CRITICAL();

      vTaskDelay(pdMS_TO_TICKS(300));
    }
  }
}

// ═══════════════════════════════════════════════════════════
//  TASK: LCD
//  State: 0=AMAN  1=BAHAYA  2=MUTED  3=LOCKED
//
//  FIX ghost-char: lcdPrintRow() selalu tulis tepat 16 char
//  → tidak perlu lcd.clear() tiap loop → tidak flicker
//  lcd.clear() hanya saat state BERGANTI
// ═══════════════════════════════════════════════════════════
void vTaskLCD(void *pvParameters) {

  // ── Tampilkan INITIALIZING ────────────────────────────
  lcd.clear();
  lcdPrintRow(0, "  INITIALIZING  ");
  lcdPrintRow(1, "   Please wait  ");
  vTaskDelay(pdMS_TO_TICKS(2000));

  // ── Tampilkan WELCOME ─────────────────────────────────
  lcd.clear();
  lcdPrintRow(0, " Lab Safety Sys ");
  lcdPrintRow(1, "    WELCOME!    ");
  vTaskDelay(pdMS_TO_TICKS(2000));

  lcd.clear();

  int lastState = -1;

  while (1) {
    ENTER_CRITICAL();
    int   adc    = sensorValue;
    int   limit  = thresholdValue;
    float temp   = tempValue;
    int   tLimit = tempThreshold;
    bool  flame  = flameDetected;
    int   fLimit = flameThreshold;
    bool  locked = systemLocked;
    bool  muted  = alarmMuted;
    EXIT_CRITICAL();

    bool danger = isDanger(adc, limit, temp, tLimit, flame, fLimit);

    // Tentukan state
    int curState;
    if      (locked)          curState = 3;
    else if (danger && muted) curState = 2;
    else if (danger)          curState = 1;
    else                      curState = 0;

    // Clear hanya saat ganti state (cegah ghost dari teks berbeda panjang)
    if (curState != lastState) {
      lcd.clear();
      lastState = curState;
    }

    // Baris 1 selalu: "GAS:xxx TMP:xx F:x"
    // Ditulis penuh 16 char via lcdPrintRow → tidak ada ghost digit
    String row1 = "GAS:";
    row1 += adc;
    row1 += " TMP:";
    row1 += (int)temp;
    row1 += " C:";
    row1 += (flame ? "Y" : "N");

    if (curState == 3) {
      // ── LOCKED ──────────────────────────────────────────
      lcdPrintRow(0, "  Sys  LOCKED   ");
      lcdPrintRow(1, "                ");

    } else if (curState == 2) {
      // ── MUTED + BAHAYA ──────────────────────────────────
      if (flame && fLimit == 1) {
        lcdPrintRow(0, "MUTED: FIRE!    ");
      } else if (temp > (float)tLimit) {
        lcdPrintRow(0, "MUTED: SUHU     ");
      } else {
        lcdPrintRow(0, "MUTED: GAS      ");
      }
      lcdPrintRow(1, row1);

    } else if (curState == 1) {
      // ── BAHAYA ──────────────────────────────────────────
      if (flame && fLimit == 1) {
        lcdPrintRow(0, "BAHAYA: FIRE!   ");
      } else if (temp > (float)tLimit) {
        lcdPrintRow(0, "BAHAYA: SUHU    ");
      } else {
        lcdPrintRow(0, "BAHAYA: GAS     ");
      }
      lcdPrintRow(1, row1);   // tetap tampilkan nilai ADC/GAS dll

    } else {
      // ── AMAN ────────────────────────────────────────────
      lcdPrintRow(0, "     AMAN       ");
      lcdPrintRow(1, row1);
    }

    vTaskDelay(pdMS_TO_TICKS(400));
  }
}

// ═══════════════════════════════════════════════════════════
//  TASK: Serial
// ═══════════════════════════════════════════════════════════
void vTaskSerial(void *pvParameters) {
  while (1) {

    // ── COMMAND DARI USB SERIAL ────────────────────────────
    if (Serial.available()) {
      String cmd = Serial.readStringUntil('\n');
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
        alarmMuted   = false;
        EXIT_CRITICAL();
      }
      else if (cmd.startsWith("set ")) {
        int value = cmd.substring(4).toInt();
        if (value >= 0 && value <= 1023) {
          ENTER_CRITICAL();
          thresholdValue = value;
          EXIT_CRITICAL();
        }
      }
    }

    // ── COMMAND DARI ESP32 ─────────────────────────────────
    if (Serial1.available()) {
      String cmd = Serial1.readStringUntil('\n');
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
        alarmMuted   = false;
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
        int value = cmd.substring(7).toInt();
        if (value >= 0 && value <= 1023) {
          ENTER_CRITICAL();
          thresholdValue = value;
          EXIT_CRITICAL();
          Serial.print("[ESP] THRESH=");
          Serial.println(value);
        }
      }
      else if (cmd.startsWith("TTEMP:")) {
        int value = cmd.substring(6).toInt();
        if (value >= 0 && value <= 100) {
          ENTER_CRITICAL();
          tempThreshold = value;
          EXIT_CRITICAL();
          Serial.print("[ESP] TTEMP=");
          Serial.println(value);
        }
      }
      else if (cmd.startsWith("TFLAME:")) {
        int value = cmd.substring(7).toInt();
        if (value == 0 || value == 1) {
          ENTER_CRITICAL();
          flameThreshold = value;
          EXIT_CRITICAL();
          Serial.print("[ESP] TFLAME=");
          Serial.println(value);
        }
      }
    }

    // ── KIRIM DATA KE ESP32 ────────────────────────────────
    ENTER_CRITICAL();
    int   adc    = sensorValue;
    int   limit  = thresholdValue;
    float temp   = tempValue;
    int   tLimit = tempThreshold;
    bool  flame  = flameDetected;
    int   fLimit = flameThreshold;
    bool  locked = systemLocked;
    bool  muted  = alarmMuted;
    EXIT_CRITICAL();

    Serial1.print("ADC:");
    Serial1.print(adc);
    Serial1.print(",LIMIT:");
    Serial1.print(limit);
    Serial1.print(",LOCK:");
    Serial1.print(locked);
    Serial1.print(",MUTE:");
    Serial1.print(muted);
    Serial1.print(",TEMP:");
    Serial1.print((int)temp);
    Serial1.print(",TLIMIT:");
    Serial1.print(tLimit);
    Serial1.print(",FLAME:");
    Serial1.print(flame ? 1 : 0);
    Serial1.print(",FLIMIT:");
    Serial1.println(fLimit);

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

// ═══════════════════════════════════════════════════════════
//  SETUP
// ═══════════════════════════════════════════════════════════
void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);

  pinMode(PIN_LED_RED,   OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_BUZZER,    OUTPUT);
  pinMode(PIN_BUTTON,    INPUT_PULLUP);
  pinMode(PIN_FLAME,     INPUT);

  attachInterrupt(
    digitalPinToInterrupt(PIN_BUTTON),
    ISR_MuteButton,
    FALLING
  );

  lcd.init();
  lcd.backlight();

  dht.begin();

  muteSemaphore = xSemaphoreCreateBinary();

  xTaskCreate(vTaskSensor, "Sensor", 192, NULL, 2, NULL);
  xTaskCreate(vTaskAlarm,  "Alarm",  256, NULL, 3, NULL);
  xTaskCreate(vTaskLCD,    "LCD",    256, NULL, 1, NULL);
  xTaskCreate(vTaskSerial, "Serial", 256, NULL, 2, NULL);

  vTaskStartScheduler();

  while (1);
}

void loop() {}
