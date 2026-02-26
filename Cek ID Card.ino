#include <SPI.h>
#include <MFRC522.h>

// ===== PIN RFID =====
#define SS_PIN 5
#define RST_PIN 4

MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200);
  SPI.begin();            // Inisialisasi SPI
  mfrc522.PCD_Init();     // Inisialisasi RFID
  Serial.println("Tempelkan kartu RFID...");
}

void loop() {
  // Cek kartu baru
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Baca data kartu
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Ambil UID kartu
  String rfidUID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) rfidUID += "0";
    rfidUID += String(mfrc522.uid.uidByte[i], HEX);
  }
  rfidUID.toUpperCase();

  // Tampilkan UID
  Serial.print("RFID UID: ");
  Serial.println(rfidUID);

  delay(1000); // Hindari pembacaan berulang terlalu cepat
}
