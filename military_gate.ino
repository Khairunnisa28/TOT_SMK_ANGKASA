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

// ===== LED =====
#define LED_MERAH 4
#define LED_HIJAU 2

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
  gateServo.write(0);

  // Ultrasonic
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // LED
  pinMode(LED_MERAH, OUTPUT);
  pinMode(LED_HIJAU, OUTPUT);

  // Status awal: merah menyala
  digitalWrite(LED_MERAH, HIGH);
  digitalWrite(LED_HIJAU, LOW);

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

  // ===== IDLE STATE =====
  digitalWrite(LED_MERAH, HIGH);
  digitalWrite(LED_HIJAU, LOW);

  // Cek kartu
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

  // ===== UID BENAR =====
  if (uid == authorizedUID) {
    Serial.println("Akses diterima");

    // LED hijau ON
    digitalWrite(LED_MERAH, LOW);
    digitalWrite(LED_HIJAU, HIGH);

    // Buka gerbang
    gateServo.write(90);

    // Tunggu objek lewat
    while (true) {
      float distance = readDistance();
      Serial.print("Jarak: ");
      Serial.println(distance);

      if (distance > 0 && distance < 15) {
        Serial.println("Objek lewat...");

        // Tunggu sampai objek menjauh
        while (readDistance() < 15) {
          delay(100);
        }

        Serial.println("Objek sudah lewat");
        break;
      }
      delay(200);
    }

    // Tutup gerbang
    gateServo.write(0);

    // Kembali ke idle (merah ON)
    digitalWrite(LED_HIJAU, LOW);
    digitalWrite(LED_MERAH, HIGH);
  }

  // ===== UID SALAH =====
  else {
    Serial.println("Akses ditolak");

    digitalWrite(LED_MERAH, HIGH);
    digitalWrite(LED_HIJAU, LOW);
  }

  delay(1000);
}
