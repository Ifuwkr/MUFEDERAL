#include <WiFi.h>
#include <esp_now.h>
#include <ESP32Servo.h>  // Değiştirilen kütüphane

#define CHANNEL 1
// Servo tanımı ve pin numarası
Servo myServo;
const int servoPin = 13;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.softAP("RX_1", "RX_1_Password", CHANNEL, 0);

  // Servo pinini başlat (ESP32Servo ile)
  myServo.setPeriodHertz(50); // 50 Hz PWM
  myServo.attach(servoPin, 500, 2500); // Min–Max mikro saniye aralığı

  // ESP-NOW başlat
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW başlatılamadı!");
    return;
  }

  // Gelen veriyi işlemek için callback fonksiyonu kaydet
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  // loop boş, çünkü veri gelince OnDataRecv çalışır
}

// Veri geldiğinde çalışacak fonksiyon
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  if (len == sizeof(int)) {
    int pwmDegeri;
    memcpy(&pwmDegeri, incomingData, sizeof(int));

    // Güvenli sınır kontrolü
    pwmDegeri = constrain(pwmDegeri, 500, 2500);

    // Servo'ya PWM değeri gönder
    myServo.writeMicroseconds(pwmDegeri);

    // Seri monitöre bilgi yaz
    Serial.print("Gelen PWM: ");
    Serial.println(pwmDegeri);
  } else {
    Serial.println("Bilinmeyen veri alındı.");
  }
}
