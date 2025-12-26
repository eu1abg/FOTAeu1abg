#include "FOTAeu1abg.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Update.h>

// Конструкторы
FOTAeu1abg::FOTAeu1abg(const char* firmwareType, int firmwareVersion)
    : _ssid(nullptr), _password(nullptr),
      _manifestURL(nullptr), _firmwareType(firmwareType),
      _currentVersion(firmwareVersion), _availableVersion(0),
      _wifiConfigured(false), _updateAvailable(false),
      _isUpdating(false), _debugEnabled(true),
      _lastCheckTime(0), _checkInterval(DEFAULT_CHECK_INTERVAL),
      _ledPin(DEFAULT_LED_PIN), _ledState(false),
      _lastLEDToggle(0) {}

FOTAeu1abg::FOTAeu1abg(const char* firmwareType, int firmwareVersion, 
                       const char* ssid, const char* password)
    : _ssid(ssid), _password(password),
      _manifestURL(nullptr), _firmwareType(firmwareType),
      _currentVersion(firmwareVersion), _availableVersion(0),
      _wifiConfigured(true), _updateAvailable(false),
      _isUpdating(false), _debugEnabled(true),
      _lastCheckTime(0), _checkInterval(DEFAULT_CHECK_INTERVAL),
      _ledPin(DEFAULT_LED_PIN), _ledState(false),
      _lastLEDToggle(0) {}

// Основные методы
void FOTAeu1abg::begin() {
    if (_debugEnabled) {
        Serial.println("\n=== FOTAeu1abg Library ===");
        Serial.print("Firmware type: ");
        Serial.println(_firmwareType);
        Serial.print("Current version: ");
        Serial.println(_currentVersion);
        Serial.print("Check interval: ");
        Serial.print(_checkInterval / 1000);
        Serial.println(" seconds");
        Serial.println("==========================\n");
    }
    
    if (_ledPin >= 0) {
        pinMode(_ledPin, OUTPUT);
        digitalWrite(_ledPin, LOW);
    }
    
    if (_wifiConfigured && _ssid && _password) {
        connectToWiFi();
    }
    
    _lastCheckTime = millis();
}

void FOTAeu1abg::handle() {
    unsigned long currentTime = millis();
    
    // Обновление индикации LED
    updateLED();
    
    // Проверка обновлений по таймеру
    if (!_isUpdating && currentTime - _lastCheckTime >= _checkInterval) {
        _lastCheckTime = currentTime;
        checkForUpdates(false);
    }
}

bool FOTAeu1abg::checkForUpdates(bool forceCheck) {
    if (_isUpdating) {
        _lastError = "Update already in progress";
        if (_debugEnabled) Serial.println("[FOTA] Update already in progress");
        return false;
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        if (!connectToWiFi()) {
            _lastError = "WiFi not connected";
            return false;
        }
    }
    
    // Проверяем достаточно ли памяти
    if (!hasEnoughMemory(MIN_FREE_HEAP)) {
        _lastError = "Not enough memory for update check";
        if (_debugEnabled) {
            Serial.print("[FOTA] Not enough memory. Free: ");
            Serial.print(getFreeHeap());
            Serial.print(", Required: ");
            Serial.println(MIN_FREE_HEAP);
        }
        return false;
    }
    
    if (_debugEnabled) Serial.println("[FOTA] Checking for updates...");
    
    String manifest = downloadManifest();
    if (manifest.length() == 0) {
        _lastError = "Failed to download manifest";
        return false;
    }
    
    if (!parseManifest(manifest)) {
        return false;
    }
    
    if (_availableVersion > _currentVersion) {
        _updateAvailable = true;
        
        if (_debugEnabled) {
            Serial.println("\n[FOTA] ====================================");
            Serial.print("[FOTA] UPDATE AVAILABLE!");
            Serial.print(" Current: v");
            Serial.print(_currentVersion);
            Serial.print(", New: v");
            Serial.println(_availableVersion);
            Serial.println("[FOTA] ====================================\n");
        }
        
        if (_updateAvailableCallback) {
            _updateAvailableCallback(_currentVersion, _availableVersion);
        }
        
        return true;
    }
    
    if (_debugEnabled) {
        Serial.print("[FOTA] No update needed. Current: v");
        Serial.print(_currentVersion);
        Serial.print(", Available: v");
        Serial.println(_availableVersion);
    }
    
    return false;
}

void FOTAeu1abg::performUpdate() {
    if (!_updateAvailable) {
        if (_debugEnabled) Serial.println("[FOTA] No update available");
        return;
    }
    
    if (_isUpdating) {
        if (_debugEnabled) Serial.println("[FOTA] Update already in progress");
        return;
    }
    
    startOTA();
}

// Настройки
void FOTAeu1abg::setManifestURL(const char* url) {
    _manifestURL = url;
    if (_debugEnabled) {
        Serial.print("[FOTA] Manifest URL set to: ");
        Serial.println(url);
    }
}

void FOTAeu1abg::setCheckInterval(unsigned long interval) {
    _checkInterval = interval;
    if (_debugEnabled) {
        Serial.print("[FOTA] Check interval set to: ");
        Serial.print(interval / 1000);
        Serial.println(" seconds");
    }
}

void FOTAeu1abg::setLEDPin(int pin) {
    _ledPin = pin;
    if (pin >= 0) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }
}

void FOTAeu1abg::setDebugEnabled(bool enabled) {
    _debugEnabled = enabled;
}

// Callback функции
void FOTAeu1abg::setOnProgressCallback(std::function<void(int percent)> callback) {
    _progressCallback = callback;
}

void FOTAeu1abg::setOnUpdateAvailableCallback(std::function<void(int currentVer, int newVer)> callback) {
    _updateAvailableCallback = callback;
}

void FOTAeu1abg::setOnUpdateStartCallback(std::function<void()> callback) {
    _updateStartCallback = callback;
}

void FOTAeu1abg::setOnUpdateCompleteCallback(std::function<void()> callback) {
    _updateCompleteCallback = callback;
}

void FOTAeu1abg::setOnUpdateErrorCallback(std::function<void(int error)> callback) {
    _updateErrorCallback = callback;
}

// Информация
int FOTAeu1abg::getCurrentVersion() const {
    return _currentVersion;
}

int FOTAeu1abg::getAvailableVersion() const {
    return _availableVersion;
}

bool FOTAeu1abg::isUpdateAvailable() const {
    return _updateAvailable;
}

bool FOTAeu1abg::isUpdating() const {
    return _isUpdating;
}

String FOTAeu1abg::getLastError() const {
    return _lastError;
}

// Утилиты
void FOTAeu1abg::restartDevice() {
    if (Serial) {
        Serial.println("[FOTA] Restarting device...");
        delay(100);
    }
    ESP.restart();
}

void FOTAeu1abg::printDeviceInfo() {
    if (!Serial) return;
    
    Serial.println("\n=== DEVICE INFORMATION ===");
    Serial.print("Chip model: ");
    Serial.println(ESP.getChipModel());
    Serial.print("CPU frequency: ");
    Serial.print(ESP.getCpuFreqMHz());
    Serial.println(" MHz");
    Serial.print("Free heap: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");
    Serial.print("Flash size: ");
    Serial.print(ESP.getFlashChipSize() / (1024 * 1024));
    Serial.println(" MB");
    Serial.print("SDK version: ");
    Serial.println(ESP.getSdkVersion());
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("WiFi RSSI: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    }
    Serial.println("===========================\n");
}

uint32_t FOTAeu1abg::getFreeHeap() {
    return ESP.getFreeHeap();
}

bool FOTAeu1abg::hasEnoughMemory(size_t requiredSize) {
    return ESP.getFreeHeap() > requiredSize;
}

// Приватные методы
bool FOTAeu1abg::connectToWiFi() {
    if (!_wifiConfigured || !_ssid || !_password) {
        _lastError = "WiFi not configured";
        return false;
    }
    
    if (_debugEnabled) {
        Serial.print("[FOTA] Connecting to WiFi: ");
        Serial.println(_ssid);
    }
    
    WiFi.begin(_ssid, _password);
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 30000) {
        delay(500);
        if (_debugEnabled) Serial.print(".");
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        _lastError = "WiFi connection failed";
        if (_debugEnabled) Serial.println("\n[FOTA] WiFi connection failed!");
        return false;
    }
    
    if (_debugEnabled) {
        Serial.println("\n[FOTA] WiFi connected!");
        Serial.print("[FOTA] IP address: ");
        Serial.println(WiFi.localIP());
    }
    
    return true;
}

String FOTAeu1abg::downloadManifest() {
    if (!_manifestURL) {
        _lastError = "Manifest URL not set";
        return "";
    }
    
    if (_debugEnabled) {
        Serial.print("[FOTA] Downloading manifest from: ");
        Serial.println(_manifestURL);
    }
    
    WiFiClientSecure client;
    client.setInsecure();
    
    HTTPClient http;
    http.begin(client, _manifestURL);
    http.setTimeout(10000);
    
    int httpCode = http.GET();
    
    if (_debugEnabled) {
        Serial.print("[FOTA] HTTP response: ");
        Serial.println(httpCode);
    }
    
    String payload = "";
    if (httpCode == HTTP_CODE_OK) {
        payload = http.getString();
        if (_debugEnabled) {
            Serial.print("[FOTA] Manifest size: ");
            Serial.print(payload.length());
            Serial.println(" bytes");
        }
    } else {
        _lastError = "HTTP error: " + String(httpCode);
        if (_debugEnabled) {
            Serial.print("[FOTA] Error: ");
            Serial.println(http.errorToString(httpCode));
        }
    }
    
    http.end();
    return payload;
}

bool FOTAeu1abg::parseManifest(const String& json) {
    if (json.length() == 0) {
        _lastError = "Empty manifest";
        return false;
    }
    
    // Проверяем достаточно ли памяти для парсинга JSON
    if (!hasEnoughMemory(2048)) {
        _lastError = "Not enough memory to parse JSON";
        return false;
    }
    
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, json);
    
    if (error) {
        _lastError = String("JSON parse error: ") + error.c_str();
        if (_debugEnabled) {
            Serial.print("[FOTA] JSON parse error: ");
            Serial.println(error.c_str());
        }
        return false;
    }
    
    const char* type = doc["type"];
    _availableVersion = doc["version"];
    const char* url = doc["url"];
    
    if (!type || !url) {
        _lastError = "Invalid manifest format";
        return false;
    }
    
    _firmwareURL = String(url);
    
    if (_debugEnabled) {
        Serial.print("[FOTA] Remote type: ");
        Serial.println(type);
        Serial.print("[FOTA] Remote version: ");
        Serial.println(_availableVersion);
        Serial.print("[FOTA] Firmware URL: ");
        Serial.println(_firmwareURL);
    }
    
    // Проверяем тип прошивки
    if (strcmp(type, _firmwareType) != 0) {
        _lastError = String("Firmware type mismatch. Expected: ") + _firmwareType + ", Got: " + type;
        if (_debugEnabled) {
            Serial.print("[FOTA] Firmware type mismatch! Expected: ");
            Serial.print(_firmwareType);
            Serial.print(", Got: ");
            Serial.println(type);
        }
        return false;
    }
    
    return true;
}

void FOTAeu1abg::startOTA() {
    _isUpdating = true;
    _updateAvailable = false;
    
    if (_debugEnabled) {
        Serial.println("\n[FOTA] ====================================");
        Serial.println("[FOTA] STARTING FIRMWARE UPDATE");
        Serial.println("[FOTA] ====================================\n");
    }
    
    if (_updateStartCallback) {
        _updateStartCallback();
    }
    
    // Индикация начала обновления
    if (_ledPin >= 0) {
        for (int i = 0; i < 5; i++) {
            digitalWrite(_ledPin, HIGH);
            delay(200);
            digitalWrite(_ledPin, LOW);
            delay(200);
        }
    }
    
    WiFiClientSecure client;
    client.setInsecure();
    
    HTTPClient http;
    http.begin(client, _firmwareURL.c_str());
    http.setTimeout(60000); // 60 секунд таймаут
    
    if (_debugEnabled) {
        Serial.print("[FOTA] Downloading firmware from: ");
        Serial.println(_firmwareURL);
    }
    
    int httpCode = http.GET();
    
    if (httpCode != HTTP_CODE_OK) {
        _lastError = String("HTTP error: ") + httpCode;
        if (_debugEnabled) {
            Serial.print("[FOTA] Failed to download firmware. HTTP code: ");
            Serial.println(httpCode);
        }
        
        if (_updateErrorCallback) {
            _updateErrorCallback(httpCode);
        }
        
        _isUpdating = false;
        http.end();
        return;
    }
    
    // Получаем размер файла
    int contentLength = http.getSize();
    if (_debugEnabled) {
        Serial.print("[FOTA] Firmware size: ");
        Serial.print(contentLength);
        Serial.println(" bytes");
    }
    
    if (contentLength <= 0) {
        _lastError = "Invalid file size";
        if (_debugEnabled) Serial.println("[FOTA] Invalid file size");
        
        if (_updateErrorCallback) {
            _updateErrorCallback(-1);
        }
        
        _isUpdating = false;
        http.end();
        return;
    }
    
    // Проверяем достаточно ли места
    if (!Update.begin(contentLength)) {
        int error = Update.getError();
        _lastError = String("Update.begin failed: ") + error;
        
        if (_debugEnabled) {
            Serial.print("[FOTA] Not enough space for update. Error: ");
            Serial.println(error);
        }
        
        if (_updateErrorCallback) {
            _updateErrorCallback(error);
        }
        
        _isUpdating = false;
        http.end();
        return;
    }
    
    if (_debugEnabled) Serial.println("[FOTA] Downloading and flashing...");
    
    // Настраиваем callback для прогресса
    Update.onProgress([this](size_t progress, size_t total) {
        int percent = (progress * 100) / total;
        
        // Вызываем callback прогресса
        if (_progressCallback) {
            _progressCallback(percent);
        }
        
        // Дебаг вывод каждые 10%
        if (_debugEnabled && percent % 10 == 0) {
            static int lastPercent = -1;
            if (percent != lastPercent) {
                lastPercent = percent;
                Serial.print("[FOTA] Progress: ");
                Serial.print(percent);
                Serial.println("%");
            }
        }
        
        // Индикация LED во время обновления
        if (_ledPin >= 0 && percent % 20 == 0) {
            digitalWrite(_ledPin, !digitalRead(_ledPin));
        }
    });
    
    // Скачиваем и записываем прошивку с буферизацией для длинных файлов
    WiFiClient* stream = http.getStreamPtr();
    size_t written = 0;
    uint8_t buffer[BUFFER_SIZE];
    
    while (http.connected() && written < contentLength) {
        size_t available = stream->available();
        if (available) {
            size_t toRead = min(available, sizeof(buffer));
            size_t read = stream->readBytes(buffer, toRead);
            
            if (Update.write(buffer, read) != read) {
                _lastError = "Write error during OTA";
                if (_debugEnabled) Serial.println("[FOTA] Write error!");
                break;
            }
            
            written += read;
        }
        delay(1);
    }
    
    if (_debugEnabled) {
        Serial.print("[FOTA] Downloaded: ");
        Serial.print(written);
        Serial.print(" of ");
        Serial.print(contentLength);
        Serial.println(" bytes");
    }
    
    if (written != contentLength) {
        _lastError = "Download incomplete";
        if (_debugEnabled) Serial.println("[FOTA] Download incomplete!");
        Update.end();
        
        if (_updateErrorCallback) {
            _updateErrorCallback(-2);
        }
        
        _isUpdating = false;
        http.end();
        return;
    }
    
    // Завершаем обновление
    if (Update.end()) {
        if (_debugEnabled) Serial.println("[FOTA] Firmware written successfully");
        
        if (Update.isFinished()) {
            if (_debugEnabled) {
                Serial.println("[FOTA] Update completed and verified!");
                Serial.println("\n[FOTA] ====================================");
                Serial.println("[FOTA] UPDATE SUCCESSFUL!");
                Serial.println("[FOTA] Device will restart");
                Serial.println("[FOTA] ====================================\n");
            }
            
            // Финальная индикация
            if (_ledPin >= 0) {
                for (int i = 0; i < 10; i++) {
                    digitalWrite(_ledPin, HIGH);
                    delay(100);
                    digitalWrite(_ledPin, LOW);
                    delay(100);
                }
            }
            
            if (_updateCompleteCallback) {
                _updateCompleteCallback();
            }
            
            delay(2000);
            ESP.restart();
        } else {
            _lastError = "Update not finished";
            if (_debugEnabled) Serial.println("[FOTA] Update not marked as finished");
            
            if (_updateErrorCallback) {
                _updateErrorCallback(-3);
            }
        }
    } else {
        int error = Update.getError();
        _lastError = String("Update.end failed: ") + error;
        
        if (_debugEnabled) {
            Serial.print("[FOTA] Update failed. Error: ");
            Serial.println(error);
        }
        
        if (_updateErrorCallback) {
            _updateErrorCallback(error);
        }
    }
    
    http.end();
    _isUpdating = false;
}

void FOTAeu1abg::updateLED() {
    if (_ledPin < 0) return;
    
    unsigned long currentTime = millis();
    
    if (_isUpdating) {
        // Быстрое мигание во время обновления
        if (currentTime - _lastLEDToggle > 500) {
            _lastLEDToggle = currentTime;
            _ledState = !_ledState;
            digitalWrite(_ledPin, _ledState);
        }
    } else if (_updateAvailable) {
        // Медленное мигание при наличии обновления
        if (currentTime - _lastLEDToggle > 1000) {
            _lastLEDToggle = currentTime;
            _ledState = !_ledState;
            digitalWrite(_ledPin, _ledState);
        }
    } else if (WiFi.status() == WL_CONNECTED) {
        // Стабильный свет при подключении
        if (!_ledState) {
            _ledState = true;
            digitalWrite(_ledPin, HIGH);
        }
    } else {
        // Выключен при отсутствии WiFi
        if (_ledState) {
            _ledState = false;
            digitalWrite(_ledPin, LOW);
        }
    }
}