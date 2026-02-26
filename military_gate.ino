#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>

// ===== RFID =====
#define SS_PIN 5
#define RST_PIN 4
MFRC522 mfrc522(SS_PIN, RST_PIN);

// ===== SERVO =====
#define SERVO_PIN 13
Servo gateServo;

// ===== ULTRASONIC =====
#define TRIG_PIN 12
#define ECHO_PIN 14

// ===== UID VALID =====
String authorizedUID = "C521C906";

//
// ===== SETUP =====
//
void setup() {
  Serial.begin(115200);

  // RFID
  SPI.begin(18, 19, 23, 5);
  mfrc522.PCD_Init();

  // Servo
  gateServo.attach(SERVO_PIN);
  gateServo.write(0); // posisi tertutup

  // Ultrasonic
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.println("Gate Ready...");
}

//
// ===== BACA JARAK =====
//
float readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1;

  float distance = duration * 0.034 / 2;
  return distance;
}

//
// ===== LOOP =====
//
void loop() {
  // Cek kartu RFID
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  // Baca UID
  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();

  Serial.print("UID: ");
  Serial.println(uid);

  // Jika kartu valid → buka gerbang
  if (uid == authorizedUID) {
    Serial.println("Akses diterima - Gerbang dibuka");
    gateServo.write(90);

    // Tunggu objek lewat
    while (true) {
      float distance = readDistance();
      Serial.print("Jarak: ");
      Serial.println(distance);

      // Jika objek terdeteksi lewat (< 15 cm)
      if (distance > 0 && distance < 15) {
        Serial.println("Objek lewat...");
        
        // Tunggu sampai objek menjauh
        while (readDistance() < 15) {
          delay(100);
        }

        Serial.println("Objek sudah lewat, gerbang menutup");
        break;
      }
      delay(200);
    }

    // Tutup gerbang
    gateServo.write(0);
    delay(1000);
  }
  else {
    Serial.println("Akses ditolak");
  }

  delay(1000);
}
