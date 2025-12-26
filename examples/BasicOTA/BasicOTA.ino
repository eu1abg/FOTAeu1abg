#include <FOTAeu1abg.h>

// Создаем объект FOTA
//FOTAeu1abg fota("esp32-fota-http", 1);
FOTAeu1abg fota("esp32-fota-http", 1,"SSID","password");

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("=== Basic OTA Example ===");
  
  // Настройка FOTA
  fota.setManifestURL("https://raw.githubusercontent.com/eu1abg/Webasto_virtuino/main/firmware/firmware.json");
  fota.setDebugEnabled(true);
  
  // Callback для прогресса
  fota.setOnProgressCallback([](int percent) {
    Serial.print("Progress: ");
    Serial.print(percent);
    Serial.println("%");
  });
  
  // Запуск FOTA
  fota.begin();
  
  // Ручная проверка обновлений
  if (fota.checkForUpdates()) {
    Serial.println("Update found! Performing update...");
    fota.performUpdate();
  }
}

void loop() {
  // Автоматическая проверка в фоновом режиме
  fota.handle();
  
  // Ваш основной код
  delay(100);
}