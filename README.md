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