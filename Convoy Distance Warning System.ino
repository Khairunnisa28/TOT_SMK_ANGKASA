// ====== DEFINISI PIN ======
#define TRIG_PIN   5
#define ECHO_PIN   18
#define LED_MERAH  2
#define LED_HIJAU  4
#define BUZZER     15

// ====== FUNGSI MEMBACA JARAK ======
float readDistance() {
  long duration;
  float distance;

  // Kirim trigger
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Baca echo dengan timeout 30ms
  duration = pulseIn(ECHO_PIN, HIGH, 30000);

  // Jika tidak ada pantulan
  if (duration == 0) {
    return -1;  // tanda error / out of range
  }

  // Hitung jarak (cm)
  distance = duration * 0.034 / 2;
  return distance;
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(LED_MERAH, OUTPUT);
  pinMode(LED_HIJAU, OUTPUT);
  pinMode(BUZZER, OUTPUT);
}

void loop() {
  float distance = readDistance();

  // ===== Jika sensor error =====
  if (distance == -1) {
    Serial.println("Sensor error / Out of range");

    // Matikan semua output
    digitalWrite(LED_MERAH, LOW);
    digitalWrite(LED_HIJAU, LOW);
    digitalWrite(BUZZER, LOW);
  }
  else {
    // Tampilkan jarak
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    // ===== Logika kontrol =====
    if (distance < 20) {
      digitalWrite(LED_MERAH, HIGH);
      digitalWrite(LED_HIJAU, LOW);
      digitalWrite(BUZZER, HIGH);
    } else {
      digitalWrite(LED_MERAH, LOW);
      digitalWrite(LED_HIJAU, HIGH);
      digitalWrite(BUZZER, LOW);
    }
  }

  delay(500);
}
