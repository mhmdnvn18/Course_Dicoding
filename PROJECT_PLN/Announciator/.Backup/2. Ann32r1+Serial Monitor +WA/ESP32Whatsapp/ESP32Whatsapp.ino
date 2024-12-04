#include <WiFi.h>      // Library untuk menghubungkan ESP32 ke jaringan WiFi
#include <HTTPClient.h> // Library untuk melakukan HTTP request
#include <UrlEncode.h>  // Library untuk melakukan URL encoding (digunakan untuk teks pesan)

#include <SoftwareSerial.h> // Menginisialisasi SoftwareSerial pada pin 16 (RX) dan 17 (TX)
SoftwareSerial mySerial(12, 13); // RX, TX

const char* ssid = "CIAMIS SOLID";    // Nama jaringan WiFi yang akan dihubungkan
const char* password = "tanpapassword"; // Password dari jaringan WiFi

// Nomor telepon dengan kode negara internasional
// Contoh: +351 untuk Portugal, +62 untuk Indonesia, dan diikuti nomor telepon
String phoneNumber = "6285175306797";

// API key dari CallMeBot, ini digunakan untuk otentikasi saat mengirim pesan WhatsApp
String apiKey = "9415931";

// Fungsi untuk mengirim pesan ke WhatsApp melalui CallMeBot API
void sendMessage(String message){

  // Membuat URL request dengan nomor telepon, API key, dan pesan yang di-URL encode
  String url = "https://api.callmebot.com/whatsapp.php?phone=6285175306797&text=This+is+a+test&apikey=9415931" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);    

  // Membuat instance dari HTTPClient untuk mengirim HTTP request
  HTTPClient http;
  http.begin(url); // Memulai HTTP request dengan URL yang sudah dibentuk

  // Menambahkan header untuk tipe konten yang digunakan dalam request
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Mengirimkan HTTP POST request dan menyimpan response code
  int httpResponseCode = http.POST(url);

  // Mengecek apakah pesan berhasil dikirim (response code 200)
  if (httpResponseCode == 200){
    Serial.print("Message sent successfully"); // Pesan berhasil dikirim
  }
  else{
    Serial.println("Error sending the message"); // Ada kesalahan saat mengirim pesan
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode); // Menampilkan kode response dari server
  }

  // Membersihkan dan menutup koneksi HTTP
  http.end();
}

void setup() {
  Serial.begin(115200); // Memulai komunikasi serial dengan baud rate 115200

  // Menginisialisasi SoftwareSerial dengan baud rate 9600
  mySerial.begin(9600);
  // Menampilkan pesan inisialisasi di Serial Monitor
  Serial.println("ESP32 SoftwareSerial Communication Started");

  // Menghubungkan ESP32 ke jaringan WiFi dengan SSID dan password yang diberikan
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  
  // Loop sampai ESP32 terhubung ke jaringan WiFi
  while(WiFi.status() != WL_CONNECTED) {
    delay(500); // Tunggu 500ms sebelum mencoba lagi
    Serial.print("."); // Tampilkan tanda proses koneksi
  }

  // Tampilkan pesan sukses jika terhubung ke WiFi
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP()); // Menampilkan alamat IP lokal ESP32

  // Mengirim pesan WhatsApp melalui fungsi sendMessage
  sendMessage("Hello from ESP32!"); // Pesan yang akan dikirim
}

void loop() {
  // Cek apakah data tersedia di SoftwareSerial
  if (mySerial.available()) {
    // Membaca data dari SoftwareSerial dan mencetaknya ke Serial Monitor
    String receivedData = mySerial.readString();
    Serial.print("Received message: ");
    Serial.println(receivedData);
//    sendMessage(receivedData); // Pesan yang akan dikirim