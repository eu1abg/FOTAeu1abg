# FOTAeu1abg Library

Библиотека для OTA (Over-The-Air) обновлений ESP32.

## Особенности

- Простое и удобное API
- Поддержка длинных прошивок (буферизация)
- Проверка свободной памяти
- Отображение прогресса в процентах
- Неблокирующая фоновая проверка
- Блокирующее мгновенное обновление
- Информация о версиях прошивки
- Callback функции для событий
- Индикация состояния через LED

## Установка

1. Скачайте библиотеку
2. Поместите в папку `libraries` вашей Arduino IDE
3. Перезапустите Arduino IDE

## Быстрый старт

```cpp
#include <FOTAeu1abg.h>

FOTAeu1abg fota("esp32-fota-http", 1, "your_SSID", "your_PASSWORD");

void setup() {
  Serial.begin(115200);
  fota.setManifestURL("https://example.com/firmware.json");
  fota.begin();
}

void loop() {
  fota.handle();
}

```
### Конструкторы
```cpp
// Без WiFi credentials (настраивается позже)
FOTAeu1abg fota("firmware-type", 1);

// С WiFi credentials
FOTAeu1abg fota("firmware-type", 1, "SSID", "password");
```

### Основные методы

begin() - инициализация библиотеки

handle() - фоновая обработка (вызывать в loop())

checkForUpdates(bool force) - проверка обновлений

performUpdate() - выполнение обновления

### Callback функции

``` setOnProgressCallback(func) - прогресс обновления

setOnUpdateAvailableCallback(func) - найдено обновление

setOnUpdateStartCallback(func) - начало обновления

setOnUpdateCompleteCallback(func) - завершение обновления

setOnUpdateErrorCallback(func) - ошибка обновления
```

### Информация

```` getCurrentVersion() - текущая версия

getAvailableVersion() - доступная версия

isUpdateAvailable() - есть ли обновление

isUpdating() - идет ли обновление

getLastError() - последняя ошибка
```

### Утилиты

```
restartDevice() - перезагрузка устройства

printDeviceInfo() - информация об устройстве

getFreeHeap() - свободная память

hasEnoughMemory(size) - проверка памяти
```

## Формат JSON манифеста

```json
{
  "type": "esp32-fota-http",
  "version": 2,
  "url": "https://example.com/firmware.bin"
}
```

