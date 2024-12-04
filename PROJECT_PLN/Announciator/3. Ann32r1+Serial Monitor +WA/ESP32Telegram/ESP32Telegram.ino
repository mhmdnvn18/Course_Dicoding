#include <WiFi.h>
#include <HTTPClient.h>

// Ganti dengan detail jaringan WiFi Anda
const char* ssid = "CIAMIS SOLID";     // Nama jaringan WiFi
const char* password = "tanpapassword"; // Password WiFi

// Ganti dengan token bot Telegram dan chat_id Anda
String botToken = "7268015060:AAG5S4OHK4eH9drXb-54dvxAin7PGr8CpwA";  // Token bot Telegram
String chatID = "6495702585";          // Chat ID penerima

// Pesan yang ingin dikirim
String message = "Halo, ini pesan dari ESP32!";

// Variabel untuk melacak waktu
unsigned long previousMillis = 0;  
const long interval = 2000;  // Interval 2 detik (2000 milidetik)

void setup() {
  // Inisialisasi Serial Monitor untuk debugging
  Serial.begin(115200);

  // Menghubungkan ke jaringan WiFi
  Serial.print("Menghubungkan ke ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // Tunggu sampai ESP32 terhubung ke WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Menghubungkan...");
  }

  Serial.println("Terhubung ke WiFi!");
}

void sendMessageToTelegram(String message) {
  if (WiFi.status() == WL_CONNECTED) { // Periksa apakah terhubung ke WiFi
    HTTPClient http;

    // Membuat URL API Telegram
    String url = "https://api.telegram.org/bot" + botToken + "/sendMessage?chat_id=" + chatID + "&text=" + message;

    http.begin(url);  // Inisialisasi HTTPClient dengan URL Telegram
    int httpCode = http.GET(); // Kirim GET request

    // Cek apakah permintaan berhasil (kode 200)
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println("Pesan berhasil dikirim!");
      Serial.println("Response dari server: ");
      Serial.println(payload);
    } else {
      Serial.println("Gagal mengirim pesan. Kode Error: " + String(httpCode));
    }

    http.end(); // Mengakhiri koneksi HTTP
  } else {
    Serial.println("Tidak terhubung ke WiFi.");
  }
}

void loop() {
  // Mendapatkan waktu saat ini
  unsigned long currentMillis = millis();

  // Periksa apakah sudah 2 detik berlalu
  if (currentMillis - previousMillis >= interval) {
    // Simpan waktu sekarang
    previousMillis = currentMillis;

    // Kirim pesan ke Telegram
    sendMessageToTelegram(message);
  }
}
