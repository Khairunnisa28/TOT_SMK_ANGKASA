#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>
#include <time.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//
// ===== KONFIGURASI =====
//
const char* ssid = "WiFi 6";
const char* password = "edogawa20";

#define BOT_TOKEN "8687645298:AAH7-QwBOXSjsAxwbBAwjTvvjUlkZ44QopM"
#define CHAT_ID   "1116828184"

// RFID
#define SS_PIN 5
#define RST_PIN 4

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

// BUZZER
#define BUZZER 15

// EEPROM
#define EEPROM_SIZE 4096
#define MAX_USERS   50

//
// ===== OBJEK =====
//
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);
MFRC522 mfrc522(SS_PIN, RST_PIN);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//
// ===== DATA USER =====
//
struct User {
  char uid[12];
  char name[20];
};

User users[MAX_USERS];
int userCount = 0;

bool registerMode = false;
String lastUID = "";

//
// ===== FUNGSI =====
//

// OLED
void showMessage(String line1, String line2="") {
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(line1);
  display.setCursor(0,20);
  display.println(line2);
  display.display();
}

// Buzzer
void beep(int dur) {
  digitalWrite(BUZZER, HIGH);
  delay(dur);
  digitalWrite(BUZZER, LOW);
}

// EEPROM
void saveUsers() {
  EEPROM.put(0, userCount);
  EEPROM.put(4, users);
  EEPROM.commit();
}

void loadUsers() {
  EEPROM.get(0, userCount);
  EEPROM.get(4, users);
  if (userCount < 0 || userCount > MAX_USERS) userCount = 0;
}

// UID reader (versi rapi seperti kode kamu)
String readUID() {
  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  return uid;
}

int findUser(String uid) {
  for (int i=0;i<userCount;i++) {
    if (uid.equals(users[i].uid)) return i;
  }
  return -1;
}

void sendAttendance(String name, String uid) {
  time_t now = time(nullptr);
  struct tm* t = localtime(&now);

  char timeStr[30];
  strftime(timeStr, sizeof(timeStr), "%d-%m-%Y %H:%M:%S", t);

  String msg = "📌 ABSENSI\n";
  msg += "Nama: " + name + "\n";
  msg += "UID: " + uid + "\n";
  msg += "Waktu: " + String(timeStr);

  bot.sendMessage(CHAT_ID, msg, "");
}

//
// ===== SETUP =====
//
void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  pinMode(BUZZER, OUTPUT);

  // OLED
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  showMessage("Hello,", "Silahkan Tap");

  // WiFi
  WiFi.begin(ssid, password);
  client.setInsecure();
  while (WiFi.status() != WL_CONNECTED) delay(500);

  // Telegram
  bot.sendMessage(CHAT_ID, "Sistem Absensi Aktif", "");

  // Time
  configTime(7*3600, 0, "pool.ntp.org");

  // RFID
  SPI.begin();
  mfrc522.PCD_Init();

  // Load EEPROM
  loadUsers();
}

//
// ===== LOOP =====
//
void loop() {
  handleTelegram();
  handleRFID();
}

//
// ===== TELEGRAM =====
//
void handleTelegram() {
  int newMsg = bot.getUpdates(bot.last_message_received + 1);

  while (newMsg) {
    String text = bot.messages[0].text;

    if (text == "/register") {
      registerMode = true;
      bot.sendMessage(CHAT_ID, "Mode daftar aktif. Tap kartu.", "");
    }
    else if (registerMode && lastUID != "") {
      if (userCount < MAX_USERS) {
        strcpy(users[userCount].uid, lastUID.c_str());
        strcpy(users[userCount].name, text.c_str());
        userCount++;
        saveUsers();

        showMessage("Terdaftar:", text);
        bot.sendMessage(CHAT_ID, "Kartu terdaftar: " + text, "");
      }
      registerMode = false;
      lastUID = "";
    }

    newMsg = bot.getUpdates(bot.last_message_received + 1);
  }
}

//
// ===== RFID =====
//
void handleRFID() {
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    return;

  String uid = readUID();

  Serial.print("UID: ");
  Serial.println(uid);

  // MODE DAFTAR
  if (registerMode) {
    lastUID = uid;
    showMessage("Masukkan nama", "di Telegram");
    beep(200);
    delay(2000);
    return;
  }

  // MODE ABSENSI
  int index = findUser(uid);

  if (index >= 0) {
    String name = users[index].name;

    showMessage("Absen Berhasil", name);
    beep(100);
    sendAttendance(name, uid);
  }
  else {
    showMessage("Kartu tidak", "dikenal");
    beep(500);
  }

  delay(2000);
  showMessage("Hello,", "Silahkan Tap");
}
