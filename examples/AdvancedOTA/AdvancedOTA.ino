#include <FOTAeu1abg.h>

// WiFi credentials
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

// Создаем объект FOTA с WiFi credentials
FOTAeu1abg fota("esp32-fota-http", 1, ssid, password);

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("=== Advanced OTA Example ===");
  
  // Настройка FOTA
  fota.setManifestURL("https://raw.githubusercontent.com/eu1abg/Webasto_virtuino/main/firmware/firmware.json");
  fota.setCheckInterval(60000); // Проверка каждую минуту
  fota.setLEDPin(2); // Индикация на пине 2
  fota.setDebugEnabled(true);
  
  // Callback функции
  fota.setOnProgressCallback(onProgress);
  fota.setOnUpdateAvailableCallback(onUpdateAvailable);
  fota.setOnUpdateStartCallback(onUpdateStart);
  fota.setOnUpdateCompleteCallback(onUpdateComplete);
  fota.setOnUpdateErrorCallback(onUpdateError);
  
  // Запуск FOTA
  fota.begin();
  
  // Показываем информацию об устройстве
  FOTAeu1abg::printDeviceInfo();
}

void loop() {
  // Обработка FOTA в фоновом режиме
  fota.handle();
  
  // Команды из Serial Monitor
  if (Serial.available()) {
    char cmd = Serial.read();
    handleCommand(cmd);
  }
  
  // Ваш основной код
  delay(10);
}

// Callback функции
void onProgress(int percent) {
  Serial.print("OTA Progress: ");
  Serial.print(percent);
  Serial.println("%");
}

void onUpdateAvailable(int currentVer, int newVer) {
  Serial.print("\nUpdate available! v");
  Serial.print(currentVer);
  Serial.print(" -> v");
  Serial.println(newVer);
  Serial.println("Send 'u' to update now");
}

void onUpdateStart() {
  Serial.println("OTA update started...");
}

void onUpdateComplete() {
  Serial.println("OTA update completed!");
}

void onUpdateError(int error) {
  Serial.print("OTA error: ");
  Serial.println(error);
}

// Обработчик команд
void handleCommand(char cmd) {
  switch (cmd) {
    case 'u':
    case 'U':
      if (fota.isUpdateAvailable()) {
        Serial.println("Starting update...");
        fota.performUpdate();
      } else {
        Serial.println("No update available");
      }
      break;
      
    case 'c':
    case 'C':
      Serial.println("Checking for updates...");
      fota.checkForUpdates(true);
      break;
      
    case 'i':
    case 'I':
      Serial.print("Current version: ");
      Serial.println(fota.getCurrentVersion());
      Serial.print("Available version: ");
      Serial.println(fota.getAvailableVersion());
      Serial.print("Update available: ");
      Serial.println(fota.isUpdateAvailable() ? "YES" : "NO");
      Serial.print("Free heap: ");
      Serial.println(FOTAeu1abg::getFreeHeap());
      break;
      
    case 'r':
    case 'R':
      Serial.println("Restarting device...");
      FOTAeu1abg::restartDevice();
      break;
  }
}