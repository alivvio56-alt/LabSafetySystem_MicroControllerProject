#define BLYNK_TEMPLATE_ID   "TMPL6ZVJsjX9t"
#define BLYNK_TEMPLATE_NAME "Lab Safety System"
#define BLYNK_AUTH_TOKEN    "In69odD6vXIr0XYNFhQiyyJUKc2dsRei"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "S24";
char pass[] = "siscerklmpk5";

HardwareSerial UnoSerial(2);

/*
 * ── Blynk Virtual Pins ────────────────────────────────────
 *  V0  : (display) ADC sensor lama
 *  V1  : (display) LIMIT/threshold ADC
 *  V2  : (display) LOCK status
 *  V3  : (display) MUTE status
 *  V4  : (button)  LOCK
 *  V5  : (display) TEMP value
 *  V6  : (button)  MUTE
 *  V7  : (slider)  Threshold ADC      — 0..1023
 *  V8  : (slider)  Threshold Suhu °C  — 0..100
 *  V9  : (slider)  Threshold Flame    — 0 (nonaktif) / 1 (aktif)
 *  V10 : (display) FLAME status
 * ──────────────────────────────────────────────────────────
 */

// ── LOCK BUTTON (V4) ──────────────────────────────────────
BLYNK_WRITE(V4)
{
  int value = param.asInt();

  if (value)
  {
    UnoSerial.println("LOCK");
    Serial.println("LOCK SYSTEM");
  }
  else
  {
    UnoSerial.println("RESET");
    Serial.println("SYSTEM ACTIVE");
  }
}

// ── MUTE BUTTON (V6) ──────────────────────────────────────
BLYNK_WRITE(V6)
{
  int value = param.asInt();

  if (value)
  {
    UnoSerial.println("MUTE");
    Serial.println("SEND MUTE");
  }
}

// ── SLIDER 1: Threshold ADC (V7) — range 0..1023 ─────────
BLYNK_WRITE(V7)
{
  int threshold = param.asInt();

  UnoSerial.print("THRESH:");
  UnoSerial.println(threshold);

  Serial.print("NEW THRESHOLD ADC = ");
  Serial.println(threshold);
}

// ── SLIDER 2: Threshold Suhu °C (V8) — range 0..100 ──────
BLYNK_WRITE(V8)
{
  int threshold = param.asInt();

  UnoSerial.print("TTEMP:");
  UnoSerial.println(threshold);

  Serial.print("NEW THRESHOLD SUHU = ");
  Serial.println(threshold);
}

// ── SLIDER 3: Threshold Flame (V9) — range 0..1 ──────────
//   0 = sensor flame diabaikan
//   1 = deteksi flame dianggap bahaya
BLYNK_WRITE(V9)
{
  int threshold = param.asInt();

  UnoSerial.print("TFLAME:");
  UnoSerial.println(threshold);

  Serial.print("NEW THRESHOLD FLAME = ");
  Serial.println(threshold);
}


void setup()
{
  Serial.begin(115200);

  UnoSerial.begin(
    9600,
    SERIAL_8N1,
    16,   // RX2
    17    // TX2
  );

  Serial.println();
  Serial.println("Connecting WiFi...");

  Blynk.begin(
    BLYNK_AUTH_TOKEN,
    ssid,
    pass
  );

  Serial.println("Blynk Connected");
}


void loop()
{
  Blynk.run();

  // ── UNO R4 -> BLYNK ─────────────────────────────────────
  if (UnoSerial.available())
  {
    String data = UnoSerial.readStringUntil('\n');
    data.trim();

    int adc, limit, lock, mute, temp, tlimit, flame, flimit;

    // Format baru:
    // ADC:%d,LIMIT:%d,LOCK:%d,MUTE:%d,TEMP:%d,TLIMIT:%d,FLAME:%d,FLIMIT:%d
    if (
      sscanf(
        data.c_str(),
        "ADC:%d,LIMIT:%d,LOCK:%d,MUTE:%d,TEMP:%d,TLIMIT:%d,FLAME:%d,FLIMIT:%d",
        &adc, &limit, &lock, &mute, &temp, &tlimit, &flame, &flimit
      ) == 8
    )
    {
      Serial.println(data);

      // Data display ke dashboard
      Blynk.virtualWrite(V0,  adc);
      Blynk.virtualWrite(V1,  limit);
      Blynk.virtualWrite(V2,  lock);
      Blynk.virtualWrite(V3,  mute);
      Blynk.virtualWrite(V5,  temp);
      Blynk.virtualWrite(V10, flame);
    }
  }
}
