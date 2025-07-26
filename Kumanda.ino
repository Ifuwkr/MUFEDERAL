#include <esp_now.h>
#include <WiFi.h>

#define CHANNEL 1

esp_now_peer_info_t slave;

// Potansiyometre pini
const int potPin = 34;
int potRaw = 0;
int potMapped = 0;

bool slaveFound = false;

// Joystick pinleri
const int VRX_PIN = 32;
const int VRY_PIN = 35;

// Gönderilecek veri yapısı
typedef struct {
  int pot; // 500 - 2500 mapped
  int x;   // -255 ile 255
  int y;   // -255 ile 255
} KontrolVerisi;

KontrolVerisi veri;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  analogSetAttenuation(ADC_11db); // ADC aralığını tam kullanmak için

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW başlatılamadı!");
    return;
  }

  if (ScanForSlave()) {
    if (!esp_now_is_peer_exist(slave.peer_addr)) {
      if (esp_now_add_peer(&slave) != ESP_OK) {
        Serial.println("Peer eklenemedi!");
        return;
      }
    }
    slaveFound = true;
    Serial.println("Peer eklendi ve bağlantı hazır.");
  } else {
    Serial.println("Alıcı (RX_1) bulunamadı!");
  }
}

void loop() {
  if (!slaveFound) return;

  // Potansiyometre değeri
  potRaw = analogRead(potPin);
  veri.pot = map(potRaw, 0, 4095, 500, 2500);

  // Joystick X ve Y
  int valueX = analogRead(VRX_PIN);
  int valueY = analogRead(VRY_PIN);
  int deadzone = 35;

  veri.x = map(valueX, 0, 4095, -255, 255);
  veri.y = map(valueY, 0, 4095, -255, 255);

  if (abs(veri.x) < deadzone) veri.x = 0;
  if (abs(veri.y) < deadzone) veri.y = 0;

  // Veri gönder
  esp_err_t result = esp_now_send(slave.peer_addr, (uint8_t *)&veri, sizeof(veri));
  if (result == ESP_OK) {
    Serial.println("PWM başarıyla gönderildi!");
  } else {
    Serial.print("Gönderme hatası: ");
    Serial.println(result);
  }

  // Seri monitör çıktısı
  Serial.print("Pot: "); Serial.print(veri.pot);
  Serial.print(" | X: "); Serial.print(veri.x);
  Serial.print(" | Y: "); Serial.println(veri.y);

  delay(300);
}

// Alıcıyı tarar ve bulursa true döner
bool ScanForSlave() {
  int16_t scanResults = WiFi.scanNetworks();
  for (int i = 0; i < scanResults; ++i) {
    String SSID = WiFi.SSID(i);
    String BSSIDstr = WiFi.BSSIDstr(i);

    if (SSID.startsWith("RX_1")) {
      int mac[6];
      if (6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x",
                      &mac[0], &mac[1], &mac[2],
                      &mac[3], &mac[4], &mac[5])) {
        for (int ii = 0; ii < 6; ++ii) {
          slave.peer_addr[ii] = (uint8_t)mac[ii];
        }
        slave.channel = CHANNEL;
        slave.encrypt = false;
        return true;
      }
    }
  }
  return false;
}
