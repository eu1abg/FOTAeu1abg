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
```cpp
begin() - инициализация библиотеки

handle() - фоновая обработка (вызывать в loop())

checkForUpdates(bool force) - проверка обновлений

performUpdate() - выполнение обновления
```

### Callback функции

```cpp
 setOnProgressCallback(func) - прогресс обновления

setOnUpdateAvailableCallback(func) - найдено обновление

setOnUpdateStartCallback(func) - начало обновления

setOnUpdateCompleteCallback(func) - завершение обновления

setOnUpdateErrorCallback(func) - ошибка обновления
```

### Информация

```cpp
 getCurrentVersion() - текущая версия

getAvailableVersion() - доступная версия

isUpdateAvailable() - есть ли обновление

isUpdating() - идет ли обновление

getLastError() - последняя ошибка

```

### Утилиты

```cpp
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
## Структура полной библиотеки:
text
FOTAeu1abg/
├── src/
│   ├── FOTAeu1abg.h
│   └── FOTAeu1abg.cpp
├── examples/
│   ├── BasicOTA/
│   │   └── BasicOTA.ino
│   ├── AdvancedOTA/
│   │   └── AdvancedOTA.ino
│   └── MinimalOTA/
│       └── MinimalOTA.ino
├── test/
│   └── test_fota.ino
├── library.properties          # Для Arduino IDE
├── library.json               # Для PlatformIO
├── keywords.txt               # Для Arduino IDE
├── .piolibdeps/               # Автоматически создается PlatformIO
├── LICENSE                    # MIT License
└── README.md                  # Документация
```

