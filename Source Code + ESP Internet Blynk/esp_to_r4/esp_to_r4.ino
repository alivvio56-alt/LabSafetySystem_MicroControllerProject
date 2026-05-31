#define BLYNK_TEMPLATE_ID "TMPL6ZVJsjX9t"
#define BLYNK_TEMPLATE_NAME "Lab Safety System"
#define BLYNK_AUTH_TOKEN "In69odD6vXIr0XYNFhQiyyJUKc2dsRei"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "AG CELL 2.4G";
char pass[] = "ag251206";

HardwareSerial UnoSerial(2);


//
// =============================
// DASHBOARD -> UNO R4
// =============================
//

// LOCK BUTTON (V4)

BLYNK_WRITE(V4)
{

  int value =
    param.asInt();

  if(value)
  {

    UnoSerial.println(
      "LOCK"
    );

    Serial.println(
      "LOCK SYSTEM"
    );

  }
  else
  {

    UnoSerial.println(
      "RESET"
    );

    Serial.println(
      "SYSTEM ACTIVE"
    );

  }

}

// MUTE BUTTON (V6)

BLYNK_WRITE(V6)
{
  int value = param.asInt();

  if(value)
  {
    UnoSerial.println("MUTE");

    Serial.println("SEND MUTE");
  }
}


// THRESHOLD SLIDER (V7)

BLYNK_WRITE(V7)
{
  int threshold =
    param.asInt();

  UnoSerial.print("THRESH:");
  UnoSerial.println(threshold);

  Serial.print("NEW THRESHOLD = ");

  Serial.println(threshold);
}



void setup()
{

  Serial.begin(115200);

  UnoSerial.begin(
    9600,
    SERIAL_8N1,
    16,      // RX2
    17       // TX2
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

  //
  // UNO R4 -> BLYNK
  //

  if(UnoSerial.available())
  {

    String data =
      UnoSerial.readStringUntil('\n');

    data.trim();

    int adc;
    int limit;
    int lock;
    int mute;

    if(
      sscanf(
        data.c_str(),
        "ADC:%d,LIMIT:%d,LOCK:%d,MUTE:%d",
        &adc,
        &limit,
        &lock,
        &mute
      ) == 4
    )
    {

      Serial.println(data);

      Blynk.virtualWrite(
        V0,
        adc
      );

      Blynk.virtualWrite(
        V1,
        limit
      );

      Blynk.virtualWrite(
        V2,
        lock
      );

      Blynk.virtualWrite(
        V3,
        mute
      );
    }
  }

}
