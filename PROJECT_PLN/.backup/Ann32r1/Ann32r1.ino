#include <EEPROM.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

void(* resetFunc) (void) = 0;

#define LED_DATAPIN 46
#define TOTAL_CH 32

#define RELAY_1 4
#define RELAY_2 5

#define TEST_BTN 45
#define MUTE_BTN 44
#define ACK_BTN 43
#define RST_BTN 42


#define STEADY 0
#define NEW_EVENT 1
#define EVENT_ACK 2

byte R_VAL= 200;
byte G_VAL= 30;
byte B_VAL= 40;


volatile byte loop_state=STEADY;
volatile boolean event_mute_status=false;
volatile boolean event_ack_status=false;
volatile boolean event_test_status=false;
volatile boolean rst_pressed=false;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(TOTAL_CH, LED_DATAPIN, NEO_GRB + NEO_KHZ800);


volatile uint32_t ch_stat_old;
volatile uint32_t ch_stat_mid;
volatile uint32_t ch_stat_new;

unsigned long blink_timing=0;
boolean blink_state=true;

boolean new_event_on=false;
#define blink_interval 500 //500ms blink

void updateStat(){
  ch_stat_new = (uint32_t(PINC)) | (uint32_t(PINA)<<8) | (uint32_t(PINF)<<16) | (uint32_t(PINK)<<24);
  ch_stat_new = ~ch_stat_new;
}

void replaceStat(){
  ch_stat_old=ch_stat_new;                                  // copy status ch_stat_new ke ch_stat_old
}

void led_test(){
  for(byte i=0; i<TOTAL_CH; i++) {                 // uji fungsi LED strip (channel 0-32)
    strip.setPixelColor(i,strip.Color(R_VAL,G_VAL,B_VAL));
  }
  strip.show();                                             // kirim data ke LED untuk menghidupkan semua LED
 
  for(byte i=0; i<TOTAL_CH; i++) {                 // uji fungsi LED strip (channel 0-32)
    strip.setPixelColor(i, strip.Color(0,0,0));             // menyiapkan data untuk mematikan LED satu persatu
    delay(25);                                              //  jeda 50ms
    strip.show();                                           // kirim data ke LED
  }
  delay(200);                                               // jeda 200ms
  
}
void setup() {                                              //
  DDRC&=0x00;                                               // konfigurasi port input announciator sbg input
  DDRA&=0x00;
  DDRF&=0x00;
  DDRK&=0x00;
  
  PORTC|=0xFF;                                              // aktifkan resistor pull-up port input announciator
  PORTA|=0xFF;
  PORTF|=0xFF;
  PORTK|=0XFF;

  delay(100);

  R_VAL=  EEPROM.read(10);
  G_VAL=  EEPROM.read(11);
  B_VAL=  EEPROM.read(12);
  
//  Serial.begin(115200);                                     // mulai koneksi serial
  updateStat();
  replaceStat();  
  strip.begin();                                            // mulai inisialisasi LED strip
  strip.show(); // Initialize all pixels to 'off'           // kirim perintah ke LED strip kondisi semua mati (R:0,G:0,B:0)
  
  pinMode(RELAY_1,OUTPUT);                                  // konfigurasi pin relay sebagai output
  pinMode(RELAY_2,OUTPUT);                                  // konfigurasi pin relay sebagai output
  pinMode(TEST_BTN,INPUT_PULLUP);                           // konfigurasi pin "TEST" sebagai input dengan pull-up aktif
  pinMode(MUTE_BTN,INPUT_PULLUP);                           // konfigurasi pin "MUTE" sebagai input dengan pull-up aktif
  pinMode(ACK_BTN,INPUT_PULLUP);                            // konfigurasi pin "ACK" sebagai input dengan pull-up aktif
  pinMode(RST_BTN,INPUT_PULLUP);                            // konfigurasi pin "RST" sebagai input dengan pull-up aktif
  digitalWrite(RELAY_1,HIGH);                                // aktifkan RELAY (relay active HIGH)
  digitalWrite(RELAY_2,HIGH);                                // aktifkan RELAY (relay active HIGH)
  
  led_test();

  
//  Serial.print("Init state:");                              // beri informasi melalui serial
//  for(byte i=0; i<TOTAL_CH; i++) {                 //untuk masing-masing channel (0-7)
//    if(1&(ch_stat_old>>i)){                            //cek apakah channel kondisi aktif?
//      Serial.print("O");                                    //jika aktif kirim 'O'
//    }
//    else{
//      Serial.print("@");                                    //jika tidak aktif kirim '@'
//    }
//  }

  
  show_events();                                            //eksekusi fungsi untuk menampilkan status channel ke LED
//  Serial.println();                                         // serial ke baris baru
  digitalWrite(RELAY_1,LOW);                               // matikan relay (relay active HIGH)
  digitalWrite(RELAY_2,LOW);                               // matikan relay (relay active HIGH)
}


void show_events(){                                         //fungsi
  for(byte i=0; i<TOTAL_CH; i++){
    if(1&(ch_stat_old>>i))strip.setPixelColor(i,strip.Color(R_VAL,G_VAL,B_VAL));
    else strip.setPixelColor(i,strip.Color(0,0,0));
  }
  strip.show();
}

void routine(){
  updateStat();
  
  if(!digitalRead(TEST_BTN)){       //Ketika TEST ditekan
    if(!digitalRead(MUTE_BTN)){       //TEST + MUTE -> RED
      R_VAL=200;
      G_VAL=0;
      B_VAL=0;
      event_test_status=false;
    }
    if(!digitalRead(ACK_BTN)){        //TEST + ACK -> WHITE
      R_VAL=200;
      G_VAL=200;
      B_VAL=200;
      event_test_status=false;
    }
    else if(!digitalRead(RST_BTN)){        //TEST + RST -> RANDOM COLOR
      if(!rst_pressed){
        rst_pressed=true;
        R_VAL=random(0,200);
        G_VAL=random(0,200);
        B_VAL=random(0,200);
        event_test_status=false;
      }
    }
    else{
      rst_pressed=false;
    }
    if(!event_test_status){
//      Serial.println("test");
      event_test_status=true;

      for(byte i=0; i<TOTAL_CH; i++){
        strip.setPixelColor(i,strip.Color(R_VAL,G_VAL,B_VAL));
      }
      strip.show();
    }
  }
  else{
    if(!digitalRead(ACK_BTN) && event_mute_status){
      loop_state=EVENT_ACK;
      event_ack_status=true;
      if(!event_test_status){
        show_events();
      }
      ch_stat_mid=0;
      digitalWrite(RELAY_1,HIGH);      
    }
    
    if(!digitalRead(RST_BTN) && event_ack_status && !rst_pressed){  //reset only doable when 
      rst_pressed=true;
      digitalWrite(RELAY_1,HIGH);      
      loop_state=STEADY;
      replaceStat(); 
      show_events();  //INSTANT RESET LED WHEN EVENT DISAPPEARING WHILE RESET
    }
    else if(!digitalRead(RST_BTN) && !rst_pressed){    //FOR IMMEDIATE RESET WHILE NEW EVENT NOT ACK'ED
      rst_pressed=true;
      ch_stat_old=ch_stat_new|ch_stat_mid;
      show_events();  //INSTANT RESET LED WHEN EVENT DISAPPEARING WHILE RESET
    }
    else{
      if(digitalRead(RST_BTN)){
        rst_pressed=false;
      }
    }
    
    if(!digitalRead(MUTE_BTN)){
      event_mute_status=true;
      digitalWrite(RELAY_1,HIGH);
    }
    
    if(event_test_status){
      EEPROM.update(10,R_VAL);
      EEPROM.update(11,G_VAL);
      EEPROM.update(12,B_VAL);
      
      event_test_status=false;
      show_events();
    }
  }
  if(ch_stat_new!=ch_stat_old){
//    Serial.println("Event Occur!");
//    Serial.print("new:");
    for(byte i=0; i<TOTAL_CH;i++){
      if((1&(ch_stat_new>>i)) > (1&(ch_stat_old>>i))){  //TRIGGERED
//        Serial.print("1");
        new_event_on=true;
      }
//      else{
//        Serial.print("0");
//      }
    }
  }
  
//  Serial.print(String(ch_stat_new,HEX));
//  Serial.print(" - ");
//  Serial.println(String(ch_stat_old,HEX));
  
  if(new_event_on){
    new_event_on=false;
    event_mute_status=false;
    event_ack_status=false;
    blink_state=false;
//    Serial.println("new event on");
    
      ch_stat_mid |= ch_stat_new & (~ch_stat_old);
      ch_stat_old |= ch_stat_new;
    
    loop_state=NEW_EVENT;
    digitalWrite(RELAY_1,LOW);
    show_events();                        //instantly display new event when blink off 
//    blink_timing=millis()+blink_interval;
  }
}


void loop() {
  switch(loop_state){
    case STEADY:{      
      if(millis()>=blink_timing){               //refresh LED string setiap interval (500ms)
        blink_timing=millis()+blink_interval;
        if(!event_test_status){
          show_events();
        }
      }
      //ch_stat_old &= ch_stat_new; //EVENT AUTO RESET IF ENABLED
      routine();
      break;
    }
    case NEW_EVENT:{
      routine();
      if(millis()>=blink_timing){
        blink_timing=millis()+blink_interval;
        blink_state=!blink_state;
        if(!event_test_status){
          if(blink_state){
            //  Serial.println("new event not yet ack'ed!");
            show_events(); 
          }
          else{
            for(byte i=0; i<TOTAL_CH; i++){
              if(1&(ch_stat_mid>>i))strip.setPixelColor(i,strip.Color(0,0,0));
            }
            strip.show();
          }
        }
      }
      break;
    }
    case EVENT_ACK:{
      routine();
    }
  }
}
