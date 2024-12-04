#include <EEPROM.h>                       // Library untuk membaca dan menulis EEPROM
#include <Adafruit_NeoPixel.h>            // Library untuk mengontrol strip LED NeoPixel
#ifdef __AVR__
  #include <avr/power.h>                  // Library untuk pengaturan daya pada mikrokontroler AVR
#endif

#include <SoftwareSerial.h>               //Library untuk menambahkan komunikasi serial
// Menginisialisasi SoftwareSerial pada pin 16 (TX) dan 17 (RX)
SoftwareSerial mySerial(17, 16); // RX, TX

// Deklarasi fungsi reset software
void(* resetFunc) (void) = 0;

// Definisi pin dan jumlah channel LED
#define LED_Indicator 47                  // Pin untuk LED indikator
#define LED_DATAPIN 46                    // Pin data untuk strip LED
#define TOTAL_CH 32                       // Jumlah total channel LED

// Definisi pin untuk relay dan tombol
#define RELAY_1 4                         // Pin untuk Relay 1
#define RELAY_2 5                         // Pin untuk Relay 2
#define TEST_BTN 45                       // Pin untuk tombol TEST
#define MUTE_BTN 44                       // Pin untuk tombol MUTE
#define ACK_BTN 43                        // Pin untuk tombol ACK
#define RST_BTN 42                        // Pin untuk tombol RESET

// Definisi status loop
#define STEADY 0                          // Status steady
#define NEW_EVENT 1                       // Status event baru
#define EVENT_ACK 2                       // Status event diakui

// Nilai warna default untuk LED (Merah, Hijau, Biru)
byte R_VAL = 200;                         // Nilai warna merah
byte G_VAL = 30;                          // Nilai warna hijau
byte B_VAL = 40;                          // Nilai warna biru

// Variabel status yang volatile (digunakan dalam ISR atau perubahan cepat)
volatile byte loop_state = STEADY;        // Status loop awal
volatile boolean event_mute_status = false; // Status mute event
volatile boolean event_ack_status = false;  // Status acknowledge event
volatile boolean event_test_status = false; // Status test event
volatile boolean rst_pressed = false;       // Status tombol reset ditekan

// Inisialisasi objek strip LED NeoPixel
Adafruit_NeoPixel strip = Adafruit_NeoPixel(TOTAL_CH, LED_DATAPIN, NEO_GRB + NEO_KHZ800);

// Variabel status channel
volatile uint32_t ch_stat_old;             // Status channel sebelumnya
volatile uint32_t ch_stat_mid;             // Status channel tengah
volatile uint32_t ch_stat_new;             // Status channel baru

// Variabel timing untuk blink LED
unsigned long blink_timing = 0;            // Waktu berikutnya untuk blink
boolean blink_state = true;                // Status blink saat ini

boolean new_event_on = false;              // Status apakah ada event baru
#define blink_interval 500 //500ms blink    // Interval blink dalam milidetik

/**
 * Fungsi untuk memperbarui status channel baru
 */
void updateStat(){
  // Membaca status dari port PINC, PINA, PINF, dan PINK, lalu menggabungkannya menjadi satu nilai 32-bit
  ch_stat_new = (uint32_t(PINC)) | (uint32_t(PINA) << 8) | (uint32_t(PINF) << 16) | (uint32_t(PINK) << 24);
  ch_stat_new = ~ch_stat_new;               // Membalikkan nilai status
}

/**
 * Fungsi untuk mengganti status lama dengan status baru
 */
void replaceStat(){
  ch_stat_old = ch_stat_new;                // Menyalin status baru ke status lama
}

/**
 * Fungsi untuk menguji LED strip dan LED indikator
 */
void led_test(){
  // Menghidupkan semua LED dengan warna yang ditentukan dan mengaktifkan LED indikator
  for(byte i = 0; i < TOTAL_CH; i++) {
    strip.setPixelColor(i, strip.Color(R_VAL, G_VAL, B_VAL));
    digitalWrite(LED_Indicator, HIGH);
  }
  strip.show();                               // Mengirim data ke LED strip untuk menghidupkan semua LED

  // Mematikan semua LED satu per satu dengan jeda 25ms
  for(byte i = 0; i < TOTAL_CH; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0)); // Menyiapkan data untuk mematikan LED
    delay(25);                                // Jeda 25ms
    strip.show();                             // Mengirim data ke LED strip
  }
  delay(200);                                 // Jeda 200ms
  digitalWrite(LED_Indicator, LOW);           // Mematikan LED indikator

}

/**
 * Fungsi setup yang dijalankan sekali saat mikrokontroler mulai
 */
void setup() {                                              
  // Mengatur semua pin PORTC, PORTA, PORTF, PORTK sebagai input
  DDRC &= 0x00;                                              
  DDRA &= 0x00;
  DDRF &= 0x00;
  DDRK &= 0x00;
  
  // Mengaktifkan resistor pull-up pada semua pin PORTC, PORTA, PORTF, PORTK
  PORTC |= 0xFF;                                              
  PORTA |= 0xFF;
  PORTF |= 0xFF;
  PORTK |= 0xFF;

  delay(100); // Jeda 100ms

  // Membaca nilai warna dari EEPROM
  R_VAL = EEPROM.read(10);
  G_VAL = EEPROM.read(11);
  B_VAL = EEPROM.read(12);
  
  // Serial.begin(115200); // Memulai koneksi serial (dikomentari)
  
  updateStat();             // Memperbarui status channel baru
  replaceStat();            // Menyalin status baru ke status lama
  strip.begin();            // Inisialisasi strip LED NeoPixel
  strip.show();             // Mengirim perintah ke strip LED untuk memastikan semua LED mati

  // Mengatur mode pin untuk relay dan tombol
  pinMode(RELAY_1, OUTPUT);          // Mengatur RELAY_1 sebagai output
  pinMode(RELAY_2, OUTPUT);          // Mengatur RELAY_2 sebagai output
  pinMode(TEST_BTN, INPUT_PULLUP);    // Mengatur TEST_BTN sebagai input dengan pull-up
  pinMode(MUTE_BTN, INPUT_PULLUP);    // Mengatur MUTE_BTN sebagai input dengan pull-up
  pinMode(ACK_BTN, INPUT_PULLUP);     // Mengatur ACK_BTN sebagai input dengan pull-up
  pinMode(RST_BTN, INPUT_PULLUP);     // Mengatur RST_BTN sebagai input dengan pull-up
  
  // Mengaktifkan relay (aktif tinggi)
  digitalWrite(RELAY_1, HIGH);                              
  digitalWrite(RELAY_2, HIGH);                              
  
  led_test();                // Menjalankan fungsi uji LED
  show_events();             // Menampilkan status event pada LED strip
  
  // Mematikan relay setelah uji
  digitalWrite(RELAY_1, LOW);                                
  digitalWrite(RELAY_2, LOW);

  Serial.begin(9600);                           // Memulai koneksi serial (harus dipindahkan ke setup untuk efisiensi)
  Serial.begin(115200);                         // Menginisialisasi Serial Monitor untuk debugging
  mySerial.begin(9600);                         // Menginisialisasi SoftwareSerial dengan baud rate 9600

  Serial.println("LED TESTED");                 // Mencetak pesan ke serial monitor
  mySerial.println("Hello from Arduino Mega");  // Menampilkan pesan yang dikirim di Serial Monitor
  Serial.println("Message sent to ESP32 via SoftwareSerial");       // Jeda 2 detik sebelum mengirim pesan lagi
  delay(2000);

}

//Program untuk menampilkan status pada serial monitor
void detectChanges() {
  for (byte i = 0; i < TOTAL_CH; i++) {
    if ((1 & (ch_stat_new >> i)) != (1 & (ch_stat_old >> i))) {  // Jika status channel berubah
      Serial.print("Channel ");
      Serial.print(i);
      Serial.print(" has changed to: ");
      Serial.println((1 & (ch_stat_new >> i)) ? "ON" : "OFF"); // Tampilkan status baru

      mySerial.print("Channel ");
      mySerial.print(i);
      mySerial.print(" has changed to: ");
      mySerial.println((1 & (ch_stat_new >> i)) ? "ON" : "OFF"); // Tampilkan status baru
    }
  }
}


/**
 * Fungsi untuk menampilkan status event pada strip LED
 */
void show_events(){                                         
  for(byte i = 0; i < TOTAL_CH; i++){
    if(1 & (ch_stat_old >> i)) 
      strip.setPixelColor(i, strip.Color(R_VAL, G_VAL, B_VAL)); // Menyalakan LED jika status channel aktif
    else 
      strip.setPixelColor(i, strip.Color(0, 0, 0));            // Mematikan LED jika status channel tidak aktif
  }
  strip.show();                                 // Mengirim data ke strip LED
  detectChanges();  // Panggil fungsi untuk mendeteksi perubahan

}

/**
 * Fungsi rutin utama untuk menangani logika tombol dan event
 */
void routine(){
  updateStat(); // Memperbarui status channel baru
  
  // Jika tombol TEST ditekan
  if(!digitalRead(TEST_BTN)){
    // Jika tombol MUTE juga ditekan
    if(!digitalRead(MUTE_BTN)){
      R_VAL = 200; // Mengatur warna merah
      G_VAL = 0;   // Mengatur warna hijau
      B_VAL = 0;   // Mengatur warna biru
      event_test_status = false; // Reset status test event
    }
    
    // Jika tombol ACK juga ditekan
    if(!digitalRead(ACK_BTN)){
      R_VAL = 200; // Mengatur warna merah
      G_VAL = 200; // Mengatur warna hijau
      B_VAL = 200; // Mengatur warna biru
      event_test_status = false; // Reset status test event
    }
    // Jika tombol RESET juga ditekan
    else if(!digitalRead(RST_BTN)){
      if(!rst_pressed){
        rst_pressed = true; // Menandai bahwa tombol reset telah ditekan
        R_VAL = random(0, 200); // Mengatur warna merah secara acak
        G_VAL = random(0, 200); // Mengatur warna hijau secara acak
        B_VAL = random(0, 200); // Mengatur warna biru secara acak
        event_test_status = false; // Reset status test event
        Serial.println("Reset Button");
        mySerial.println("Reset Button");
      }
    }
    else{
      rst_pressed = false; // Reset status tombol reset jika tidak ditekan
    }
    
    // Jika belum ada event test
    if(!event_test_status){
      // Serial.println("test"); // Mencetak pesan ke serial monitor (dikomentari)
      event_test_status = true; // Menandai bahwa event test telah aktif

      // Mengatur semua LED dengan warna yang ditentukan
      for(byte i = 0; i < TOTAL_CH; i++){
        strip.setPixelColor(i, strip.Color(R_VAL, G_VAL, B_VAL));
      }
      strip.show(); // Mengirim data ke strip LED
    }
  }
  else{
    // Jika tombol ACK ditekan dan status mute aktif
    if(!digitalRead(ACK_BTN) && event_mute_status){
      loop_state = EVENT_ACK;        // Mengubah status loop ke EVENT_ACK
      event_ack_status = true;       // Menandai bahwa event telah diakui
      
      if(!event_test_status){
        show_events();                // Menampilkan status event pada LED strip
      }
      
      ch_stat_mid = 0;                // Reset status channel tengah
      digitalWrite(RELAY_1, HIGH);    // Mengaktifkan relay 1
    }
    
    // Jika tombol RESET ditekan dan status acknowledge aktif serta belum reset ditekan
    if(!digitalRead(RST_BTN) && event_ack_status && !rst_pressed){
      rst_pressed = true;             // Menandai bahwa tombol reset telah ditekan
      digitalWrite(RELAY_1, HIGH);    // Mengaktifkan relay 1
      loop_state = STEADY;            // Mengubah status loop ke STEADY
      replaceStat();                   // Menyalin status baru ke status lama
      show_events();                   // Menampilkan status event pada LED strip secara instan
    }
    // Jika tombol RESET ditekan dan belum ada event yang diakui
    else if(!digitalRead(RST_BTN) && !rst_pressed){
      rst_pressed = true;             // Menandai bahwa tombol reset telah ditekan
      ch_stat_old = ch_stat_new | ch_stat_mid; // Menggabungkan status channel baru dan tengah ke status lama
      show_events();                   // Menampilkan status event pada LED strip secara instan
    }
    else{
      if(digitalRead(RST_BTN)){
        rst_pressed = false;          // Mengatur ulang status tombol reset jika tidak ditekan
      }
    }
    
    // Jika tombol MUTE ditekan
    if(!digitalRead(MUTE_BTN)){
      event_mute_status = true;        // Menandai bahwa event telah dimute
      digitalWrite(RELAY_1, HIGH);     // Mengaktifkan relay 1
    }
    
    // Jika ada event test yang aktif
    if(event_test_status){
      // Menyimpan nilai warna ke EEPROM
      EEPROM.update(10, R_VAL);
      EEPROM.update(11, G_VAL);
      EEPROM.update(12, B_VAL);
      
      event_test_status = false;       // Reset status test event
      show_events();                    // Menampilkan status event pada LED strip
    }
  }
  
  // Jika status channel baru berbeda dengan status lama
  if(ch_stat_new != ch_stat_old){
    // Serial.println("Event Occur!"); // Mencetak pesan ke serial monitor (dikomentari)
    // Serial.print("new:");
    for(byte i = 0; i < TOTAL_CH; i++){
      if((1 & (ch_stat_new >> i)) > (1 & (ch_stat_old >> i))){ // Jika channel baru aktif
        // Serial.print("1"); // Mencetak '1' ke serial monitor (dikomentari)
        new_event_on = true;       // Menandai bahwa ada event baru
      }
      // else{
      //   Serial.print("0"); // Mencetak '0' ke serial monitor (dikomentari)
      // }
    }
  }
  
  // Serial.print(String(ch_stat_new, HEX)); // Mencetak status channel baru dalam format HEX (dikomentari)
  // Serial.print(" - ");
  // Serial.println(String(ch_stat_old, HEX)); // Mencetak status channel lama dalam format HEX (dikomentari)
  
  // Jika ada event baru
  if(new_event_on){
    new_event_on = false;               // Reset status event baru
    event_mute_status = false;          // Reset status mute event
    event_ack_status = false;           // Reset status acknowledge event
    blink_state = false;                // Reset status blink
    
    // Serial.println("new event on"); // Mencetak pesan ke serial monitor (dikomentari)
    
    ch_stat_mid |= ch_stat_new & (~ch_stat_old); // Menambahkan status channel baru yang belum diakui ke status tengah
    ch_stat_old |= ch_stat_new;                  // Menambahkan status channel baru ke status lama
    
    loop_state = NEW_EVENT;                      // Mengubah status loop ke NEW_EVENT
    digitalWrite(RELAY_1, LOW);                  // Menonaktifkan relay 1
    show_events();                                // Menampilkan status event pada LED strip secara instan
    // blink_timing = millis() + blink_interval;  // Mengatur waktu berikutnya untuk blink (dikomentari)
  }
}

/**
 * Fungsi loop utama yang dijalankan berulang kali
 */
void loop() {
  switch(loop_state){
    case STEADY:{      
      // Jika waktu saat ini mencapai atau melewati waktu blink berikutnya
      if(millis() >= blink_timing){
        blink_timing = millis() + blink_interval; // Mengatur waktu blink berikutnya
        if(!event_test_status){
          show_events();                          // Menampilkan status event pada LED strip
        }
      }
      routine(); // Menjalankan fungsi rutin utama
      break;
    }
    case NEW_EVENT:{
      routine(); // Menjalankan fungsi rutin utama
      // Jika waktu saat ini mencapai atau melewati waktu blink berikutnya
      if(millis() >= blink_timing){
        blink_timing = millis() + blink_interval; // Mengatur waktu blink berikutnya
        blink_state = !blink_state;               // Mengubah status blink
        
        if(!event_test_status){
          if(blink_state){
            // Serial.println("new event not yet ack'ed!"); // Mencetak pesan ke serial monitor (dikomentari)
            show_events();                          // Menampilkan status event pada LED strip
          }
          else{
            // Mematikan LED yang sedang aktif pada ch_stat_mid
            for(byte i = 0; i < TOTAL_CH; i++){
              if(1 & (ch_stat_mid >> i))
                strip.setPixelColor(i, strip.Color(0, 0, 0)); // Mematikan LED
            }
            strip.show();                             // Mengirim data ke strip LED
          }
        }
      }
      break;
    }
    case EVENT_ACK:{
      routine(); // Menjalankan fungsi rutin utama
      break;
    }
  }



}
