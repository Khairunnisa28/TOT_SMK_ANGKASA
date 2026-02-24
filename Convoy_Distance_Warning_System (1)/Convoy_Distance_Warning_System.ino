// =======================
// Deklarasi Pin
// =======================
const int TRIG_PIN   = 5;
const int ECHO_PIN   = 18;

const int LED_MERAH  = 2;
const int LED_HIJAU  = 4;
const int BUZZER     = 15;

// =======================
// Variabel Global
// =======================
long durasi;
float distance;

// =======================
// Setup
// =======================
void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(LED_MERAH, OUTPUT);
  pinMode(LED_HIJAU, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(TRIG_PIN, LOW); // pastikan trigger LOW di awal
}

// =======================
// Fungsi Membaca Jarak
// =======================
float bacaJarak() {

  // Reset Trigger
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // Kirim pulsa 10 microsecond
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Baca durasi pantulan (timeout 30ms)
  long durasi = pulseIn(ECHO_PIN, HIGH, 30000);

  // Jika tidak terbaca
  if (durasi == 0) {
    return -1;
  }

  // Hitung jarak (cm)
  float jarak = durasi * 0.034 / 2;

  return jarak;
}

// =======================
// Loop Utama
// =======================
void loop() {

  distance = bacaJarak();

  // Jika sensor gagal membaca
  if (distance < 0) {
    Serial.println("Sensor tidak terbaca");
    delay(500);
    return;
  }

  // Tampilkan jarak
  Serial.print("Jarak: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Logika indikator
  if (distance < 20) {
    digitalWrite(LED_MERAH, HIGH);
    digitalWrite(LED_HIJAU, LOW);
    digitalWrite(BUZZER, HIGH);
  } 
  else {
    digitalWrite(LED_MERAH, LOW);
    digitalWrite(LED_HIJAU, HIGH);
    digitalWrite(BUZZER, LOW);
  }

  delay(100);
}