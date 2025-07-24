#include <esp_now.h>
#include <WiFi.h>

#define CHANNEL 1

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.softAP("RX_1", "RX_1_Password", CHANNEL, 0);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW başlatılamadı!");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  // pasif
}

void OnDataRecv(const esp_now_recv_info_t *, const uint8_t *data, int data_len) {
  if (data_len == sizeof(int)) {
    int alinanVeri;
    memcpy(&alinanVeri, data, sizeof(int));
    Serial.print("Gelen integer -> ");
    Serial.println(alinanVeri);
  }
}
