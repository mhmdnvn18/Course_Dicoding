#include <SoftwareSerial.h>

// Menginisialisasi SoftwareSerial pada pin yang diinginkan, misal RX: 12 dan TX: 13
SoftwareSerial mySerial(12, 13); // RX, TX

void setup() {
  // Menginisialisasi Serial Monitor untuk debugging
  Serial.begin(115200);

  // Menginisialisasi SoftwareSerial dengan baud rate 9600
  mySerial.begin(9600);

  // Menampilkan pesan inisialisasi di Serial Monitor
  Serial.println("ESP32 SoftwareSerial Communication Started");
}

void loop() {
  // Cek apakah data tersedia di SoftwareSerial
  if (mySerial.available()) {
    // Membaca data dari SoftwareSerial dan mencetaknya ke Serial Monitor
    String receivedData = mySerial.readString();
    Serial.print("Received message: ");
    Serial.println(receivedData);
  }

  // Menambahkan jeda singkat agar loop tidak terlalu cepat
  delay(500);
}
