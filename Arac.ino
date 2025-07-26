#include <WiFi.h>
#include <esp_now.h>
#include <ESP32Servo.h>

#define RPWM1 26 // 1 sağ tekerler 2 sol tekerler
#define LPWM1 27
#define RPWM2 19
#define LPWM2 18

#define EN1 12     // 1 2 sağ 3 4 sol
#define EN2 13
#define EN3 22
#define EN4 23

Servo myServo;
const int servoPin = 13;

// Gönderici ile aynı veri yapısı tanımı
typedef struct {
  int pot;
  int x;
  int y;
} KontrolVerisi;

KontrolVerisi gelenVeri;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);

  myServo.setPeriodHertz(50); // 50 Hz PWM
  myServo.attach(servoPin, 500, 2500); // Min–Max mikro saniye aralığı

  pinMode(RPWM1, OUTPUT);
  pinMode(LPWM1, OUTPUT);
  pinMode(RPWM2, OUTPUT);
  pinMode(LPWM2, OUTPUT);
  pinMode(EN1, OUTPUT);
  pinMode(EN2, OUTPUT);
  pinMode(EN3, OUTPUT);
  pinMode(EN4, OUTPUT);

  digitalWrite(EN1, HIGH);
  digitalWrite(EN2, HIGH);
  digitalWrite(EN3, HIGH);
  digitalWrite(EN4, HIGH);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW başlatılamadı!");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);

  // İsteğe bağlı: Bu cihazı tanımlayıcı bir SSID ile yayınla (gönderici bunu tarayıp bulur)
  WiFi.setHostname("RX_1");
}

void loop() {
  // loop içinde işlem yapılmayacak, veriler callback ile alınacak
}

// Veri geldiğinde çağrılacak fonksiyon
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len == sizeof(KontrolVerisi)) {
    memcpy(&gelenVeri, data, sizeof(KontrolVerisi));

    // Verileri yazdır
    Serial.print("Potansiyometre: "); Serial.print(gelenVeri.pot);
    Serial.print(" | X: "); Serial.print(gelenVeri.x);
    Serial.print(" | Y: "); Serial.println(gelenVeri.y);
  } else {
    Serial.println("Hatalı veri boyutu alındı.");
  }
  
  hareket(gelenVeri.x, gelenVeri.y);
  servo(gelenVeri.pot);
}

void hareket(int x, int y) {
  int sag = x + y;
  int sol = y - x;

  if(sag>0){
    if(sol>0){

      analogWrite(RPWM1, abs(sag)); analogWrite(LPWM1, 0);
      analogWrite(RPWM2, 0); analogWrite(LPWM2, abs(sol));

    }else{

      analogWrite(RPWM1, abs(sag)); analogWrite(LPWM1, 0);
      analogWrite(RPWM2, abs(sol)); analogWrite(LPWM2, 0);

    }
  }else{
    if(sol>0){

      analogWrite(RPWM1, 0); analogWrite(LPWM1, abs(sag));
      analogWrite(RPWM2, 0); analogWrite(LPWM2, abs(sol));

    }else{

      analogWrite(RPWM1, 0); analogWrite(LPWM1, abs(sag));
      analogWrite(RPWM2, abs(sol)); analogWrite(LPWM2, 0);

    }
  }
}

void servo(int pot) {
  pot = constrain(pot, 500, 2500);

  myServo.writeMicroseconds(pot);
  Serial.print("Gelen PWM: ");
  Serial.println(pot);
}
