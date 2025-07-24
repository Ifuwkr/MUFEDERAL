#include <esp_now.h>
#include <WiFi.h>

#define CHANNEL 1

esp_now_peer_info_t slave;
int data = 0;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW başlatılamadı!");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  ScanForSlave();
  esp_now_add_peer(&slave);
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');  // Enter'a kadar oku
    int sayi = input.toInt();                     // Sayıya çevir
    esp_now_send(slave.peer_addr, (uint8_t *)&sayi, sizeof(sayi));
  }
}


void OnDataSent(const wifi_tx_info_t *, esp_now_send_status_t) {
  // Sessiz mod
}

void ScanForSlave() {
  int16_t scanResults = WiFi.scanNetworks();
  for (int i = 0; i < scanResults; ++i) {
    String SSID = WiFi.SSID(i);
    String BSSIDstr = WiFi.BSSIDstr(i);

    if (SSID.indexOf("RX") == 0) {
      int mac[6];
      if (6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x",
                      &mac[0], &mac[1], &mac[2],
                      &mac[3], &mac[4], &mac[5])) {
        for (int ii = 0; ii < 6; ++ii) {
          slave.peer_addr[ii] = (uint8_t)mac[ii];
        }
      }

      slave.channel = CHANNEL;
      slave.encrypt = 0;
      break;
    }
  }
}
