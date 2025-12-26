#include <FOTAeu1abg.h>

FOTAeu1abg fota("esp32-fota-http", 1, "your_SSID", "your_PASSWORD");

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  fota.setManifestURL("https://raw.githubusercontent.com/eu1abg/Webasto_virtuino/main/firmware/firmware.json");
  fota.begin();
}

void loop() {
  fota.handle(); // Фоновая проверка обновлений
  delay(100);
}