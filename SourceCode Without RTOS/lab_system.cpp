#include <Wire.h>                // Library komunikasi I2C
#include <LiquidCrystal_I2C.h>   // Library LCD I2C

// I2C LCD 
LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD alamat 0x27 ukuran 16x2

// Pin 
const int pinSensor   = A0;   // Pin sensor analog
const int pinBuzzer   = 9;    // Pin buzzer PWM
const int pinLEDMerah = 13;   // Pin LED merah
const int pinLEDHijau = 12;   // Pin LED hijau
const int pinButton   = 2;    // Pin tombol interrupt

// Threshold (diubah via "set N")
int THRESHOLD = 100;          // Nilai ambang bahaya awal

// State 
bool systemLock = false;      // Status sistem terkunci / tidak
bool muted      = false;      // Status buzzer dimatikan / tidak

void setup() {
  // UART
  Serial.begin(9600);         						   // Mulai komunikasi serial 9600 baud
  Serial.println("=== Lab Safety System ===");         // Judul sistem
  Serial.println("Commands: on | lock | set [angka]"); // Daftar perintah

  // GPIO
  pinMode(pinLEDMerah, OUTPUT); 	  // LED merah sebagai output
  pinMode(pinLEDHijau, OUTPUT); 	  // LED hijau sebagai output
  pinMode(pinBuzzer,   OUTPUT); 	  // Buzzer sebagai output
  pinMode(pinButton,   INPUT_PULLUP); // Tombol input pull-up internal

  // Interrupt – tombol untuk mute alarm
  attachInterrupt(digitalPinToInterrupt(pinButton), handleMute, FALLING);
  // Saat tombol ditekan, jalankan fungsi handleMute

  // I2C LCD init
  lcd.init();                						  // Inisialisasi LCD
  lcd.backlight();          						  // Nyalakan lampu LCD
  lcd.setCursor(0, 0); lcd.print("Lab Safety Syst "); // Baris 1
  lcd.setCursor(0, 1); lcd.print("  Initializing  "); // Baris 2
  delay(1500);              						  // Delay 1,5 detik
  lcd.clear();              						  // Bersihkan LCD
}

void loop() {
  // 1. Baca perintah UART 
  if (Serial.available() > 0) { 			   // Jika ada data masuk
    String cmd = Serial.readStringUntil('\n'); // Baca sampai enter
    cmd.trim();                 			   // Hapus spasi awal/akhir
    cmd.toLowerCase();          			   // Ubah jadi huruf kecil

    if (cmd == "lock") {      				   // Jika perintah lock
      systemLock = true;        			   // Sistem dikunci
      noTone(pinBuzzer);        			   // Matikan buzzer
      digitalWrite(pinLEDMerah, LOW); 		   // Matikan LED merah
      digitalWrite(pinLEDHijau, LOW); 		   // Matikan LED hijau
      Serial.println("[UART] Sistem DIKUNCI.");

    } else if (cmd == "on") {   // Jika perintah on
      systemLock = false;       // Sistem aktif
      muted      = false;       // Reset mute
      Serial.println("[UART] Sistem DIAKTIFKAN.");

    } else if (cmd.startsWith("set ")) { 	// Jika perintah set nilai
      int nilai = cmd.substring(4).toInt(); // Ambil angka setelah set
      if (nilai >= 0 && nilai <= 1023) {    // Jika valid
        THRESHOLD = nilai;       			// Ubah threshold
        Serial.println("[SET] Threshold: " + String(THRESHOLD));
      } else {
        Serial.println("[SET] Nilai harus 0-1023.");
      }

    } else {
      Serial.println("[UART] Command tidak dikenal: " + cmd);
    }
  }

  // 2. Jika sistem dikunci 
  if (systemLock) {
    noTone(pinBuzzer);           						// Matikan buzzer
    digitalWrite(pinLEDMerah, LOW); 					// Matikan LED merah
    digitalWrite(pinLEDHijau, LOW); 					// Matikan LED hijau
    lcd.setCursor(0, 1); lcd.print("System LOCKED   "); // Tampil status
    delay(200);                 						// Delay singkat
    return;                     						// Keluar dari loop sekarang
  }

  // 3. Baca ADC
  int nilaiSensor = analogRead(pinSensor); // Baca sensor 0-1023
  Serial.print("[ADC] Kadar Asap: ");
  Serial.println(nilaiSensor);  		   // Tampilkan nilai sensor

  lcd.setCursor(0, 0); lcd.print("Lab Safety Syst "); // Judul LCD

  // 4. Percabangan kondisi
  if (nilaiSensor > THRESHOLD && !muted) {
    // Jika bahaya dan belum mute
    digitalWrite(pinLEDMerah, HIGH); // LED merah nyala
    digitalWrite(pinLEDHijau, LOW);  // LED hijau mati
    lcd.setCursor(0, 1);
    lcd.print("BAHAYA! Val:");
    lcd.print(nilaiSensor);
    lcd.print("  ");
    Serial.println("[ALARM] BAHAYA! Val=" + String(nilaiSensor));

    // PWM sweep naik
    for (int f = 600; f <= 1200; f += 20) {
      if (muted || systemLock) break; // Stop jika mute/lock
      tone(pinBuzzer, f);             // Nada naik
      delay(8);                       // Delay singkat
    }

    // PWM sweep turun
    for (int f = 1200; f >= 600; f -= 20) {
      if (muted || systemLock) break; // Stop jika mute/lock
      tone(pinBuzzer, f);             // Nada turun
      delay(8);						  // Delay singkat
    }

  } else if (nilaiSensor > THRESHOLD && muted) {
    // Jika bahaya tapi mute aktif
    digitalWrite(pinLEDMerah, HIGH); // LED merah nyala
    digitalWrite(pinLEDHijau, LOW);  // LED hijau mati
    noTone(pinBuzzer);               // Buzzer mati
    lcd.setCursor(0, 1);
    lcd.print("Muted  Val:");
    lcd.print(nilaiSensor);
    lcd.print("   ");
    Serial.println("[MUTED] Buzzer dibungkam. Val=" + String(nilaiSensor));
    delay(300);

  } else {
    // Jika aman
    muted = false;                   // Reset mute
    noTone(pinBuzzer);               // Buzzer mati
    digitalWrite(pinLEDMerah, LOW);  // LED merah mati
    digitalWrite(pinLEDHijau, HIGH); // LED hijau nyala
    lcd.setCursor(0, 1);
    lcd.print("Aman  Val:");
    lcd.print(nilaiSensor);
    lcd.print("    ");
    delay(300);
  }
}

// ISR – Interrupt Service Routine tombol untuk mute alarm
void handleMute() {
  muted = true; // Saat tombol ditekan, buzzer di-mute
}