#include <SoftwareSerial.h>

// Menginisialisasi SoftwareSerial pada pin 16 (TX) dan 17 (RX)
SoftwareSerial mySerial(17, 16); // RX, TX

void setup() {
  // Menginisialisasi Serial Monitor untuk debugging
  Serial.begin(115200);
  
  // Menginisialisasi SoftwareSerial dengan baud rate 9600
  mySerial.begin(9600);

  // Menampilkan pesan inisialisasi di Serial Monitor
  Serial.println("Arduino Mega SoftwareSerial Communication Started");
}

void loop() {
  // Mengirim pesan ke ESP32 melalui SoftwareSerial
  mySerial.println("Hello from Arduino Mega");

  // Menampilkan pesan yang dikirim di Serial Monitor
  Serial.println("Message sent to ESP32 via SoftwareSerial");

  // Jeda 2 detik sebelum mengirim pesan lagi
  delay(2000);
}
