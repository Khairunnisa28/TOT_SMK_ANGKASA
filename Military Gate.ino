#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "Military Gate"
#define BLYNK_AUTH_TOKEN "YOUR_AUTH_TOKEN"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>
c:\Users\maleo\Downloads\microbitdht_copy_20260226093859\microbitdht_copy_20260226093859.ino
// WiFi
char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";

// Pin RFID
#define SS_PIN 5
#define RST_PIN 22
MFRC522 rfid(SS_PIN, RST_PIN);

// Pin Output
#define SERVO_PIN 13
#define LED_HIJAU 2
#define LED_MERAH 4
#define BUZZER 15

Servo gateServo;

// UID kartu yang diizinkan
byte authorizedUID[4] = {0xDE, 0xAD, 0xBE, 0xEF};

void setup() {
  Serial.begin(115200);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  SPI.begin();
  rfid.PCD_Init();

  pinMode(LED_HIJAU, OUTPUT);
  pinMode(LED_MERAH, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  gateServo.attach(SERVO_PIN);
  gateServo.write(0); // posisi awal tertutup

  Serial.println("Military Smart Gate Barrier Ready");
}

void loop() {
  Blynk.run();

  // Cek kartu
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  Serial.print("UID: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  if (checkAccess(rfid.uid.uidByte)) {
    aksesDiterima();
  } else {
    aksesDitolak();
  }

  rfid.PICC_HaltA();
}

// Cek UID
bool checkAccess(byte *uid) {
  for (byte i = 0; i < 4; i++) {
    if (uid[i] != authorizedUID[i]) {
      return false;
    }
  }
  return true;
}

// Akses diterima
void aksesDiterima() {
  Serial.println("Akses Diterima");

  digitalWrite(LED_HIJAU, HIGH);
  tone(BUZZER, 1000);
  delay(200);
  noTone(BUZZER);

  Blynk.virtualWrite(V0, "AKSES DITERIMA");

  // Buka gerbang
  gateServo.write(90);
  delay(3000);

  // Tutup kembali
  gateServo.write(0);
  digitalWrite(LED_HIJAU, LOW);
}

// Akses ditolak
void aksesDitolak() {
  Serial.println("Akses Ditolak");

  digitalWrite(LED_MERAH, HIGH);

  for (int i = 0; i < 2; i++) {
    tone(BUZZER, 500);
    delay(200);
    noTone(BUZZER);
    delay(200);
  }

  Blynk.virtualWrite(V0, "AKSES DITOLAK");
  Blynk.logEvent("alert", "Percobaan akses tidak sah");

  digitalWrite(LED_MERAH, LOW);
}