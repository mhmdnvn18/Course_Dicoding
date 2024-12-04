#include <WiFi.h>
#include <HTTPClient.h>

// Ganti dengan detail jaringan WiFi Anda
const char* ssid = "CIAMIS SOLID";    // Nama jaringan WiFi yang akan dihubungkan
const char* password = "tanpapassword"; // Password dari jaringan WiFi

// Ganti dengan token bot Telegram dan chat_id Anda
String botToken = "7268015060:AAG5S4OHK4eH9drXb-54dvxAin7PGr8CpwA";  // Masukkan token bot Telegram
String chatID = "6495702585";      // Masukkan chat_id penerima

// Pesan yang ingin dikirim
String message = "Halo, ini pesan dari ESP32!";

void setup() {
  // Inisialisasi Serial Monitor untuk debugging
  Serial.begin(115200);

  // Menghubungkan ke jaringan WiFi
  Serial.print("Menghubungkan ke ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
}
  // Tunggu sampai ESP32 terhub
