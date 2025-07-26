#include <esp_now.h>
#include <WiFi.h>

#define CHANNEL 1

esp_now_peer_info_t slave;

// Potansiyometre pini ve değişkenleri
const int potPin = 34;
int potRaw = 0;
int potMapped = 0;

bool slaveFound = false;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

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

  potRaw = analogRead(potPin);
  potMapped = map(potRaw, 0, 4095, 500, 2500);

  Serial.print("Raw: ");
  Serial.print(potRaw);
  Serial.print(" | Mapped: ");
  Serial.println(potMapped);

  esp_err_t result = esp_now_send(slave.peer_addr, (uint8_t *)&potMapped, sizeof(potMapped));
  if (result == ESP_OK) {
    Serial.println("PWM başarıyla gönderildi!");
  } else {
    Serial.print("Gönderme hatası: ");
    Serial.println(result);
  }

  delay(500);
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
        slave.encrypt = 0;
        return true;
      }
    }
  }
  return false;
}
