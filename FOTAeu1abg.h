#ifndef FOTAeu1abg_H
#define FOTAeu1abg_H

#include <Arduino.h>
#include <functional>

class FOTAeu1abg {
public:
    // Конструкторы
    FOTAeu1abg(const char* firmwareType, int firmwareVersion);
    FOTAeu1abg(const char* firmwareType, int firmwareVersion, const char* ssid, const char* password);
    
    // Основные методы
    void begin();
    void handle(); // Неблокирующая проверка в loop()
    bool checkForUpdates(bool forceCheck = false); // Блокирующая проверка
    void performUpdate(); // Блокирующее обновление
    
    // Настройки
    void setManifestURL(const char* url);
    void setCheckInterval(unsigned long interval); // Интервал проверки в миллисекундах
    void setLEDPin(int pin);
    void setDebugEnabled(bool enabled);
    
    // Callback функции
    void setOnProgressCallback(std::function<void(int percent)> callback);
    void setOnUpdateAvailableCallback(std::function<void(int currentVer, int newVer)> callback);
    void setOnUpdateStartCallback(std::function<void()> callback);
    void setOnUpdateCompleteCallback(std::function<void()> callback);
    void setOnUpdateErrorCallback(std::function<void(int error)> callback);
    
    // Информация
    int getCurrentVersion() const;
    int getAvailableVersion() const;
    bool isUpdateAvailable() const;
    bool isUpdating() const;
    String getLastError() const;
    
    // Утилиты
    static void restartDevice();
    static void printDeviceInfo();
    static uint32_t getFreeHeap();
    static uint32_t getFreePsram(); // Новый метод для PSRAM
    static bool hasEnoughMemory(size_t requiredSize);
    
private:
    // Приватные методы
    bool connectToWiFi();
    String downloadManifest();
    bool parseManifest(const String& json);
    void startOTA();
    void updateLED();
    
    // Callback функции
    std::function<void(int percent)> _progressCallback = nullptr;
    std::function<void(int currentVer, int newVer)> _updateAvailableCallback = nullptr;
    std::function<void()> _updateStartCallback = nullptr;
    std::function<void()> _updateCompleteCallback = nullptr;
    std::function<void(int error)> _updateErrorCallback = nullptr;
    
    // Конфигурация
    const char* _ssid;
    const char* _password;
    const char* _manifestURL;
    const char* _firmwareType;
    int _currentVersion;
    int _availableVersion;
    String _firmwareURL;
    
    // Состояние
    bool _wifiConfigured;
    bool _updateAvailable;
    bool _isUpdating;
    bool _debugEnabled;
    unsigned long _lastCheckTime;
    unsigned long _checkInterval;
    int _ledPin;
    bool _ledState;
    unsigned long _lastLEDToggle;
    String _lastError;
    
    // Константы
    static const unsigned long DEFAULT_CHECK_INTERVAL = 300000; // 5 минут
    static const int DEFAULT_LED_PIN = 2;
    static const size_t MIN_FREE_HEAP = 10240; // 10KB минимально свободной памяти
    // Убрана константа BUFFER_SIZE из заголовочного файла
    // Она теперь определена только в .cpp файле
};

#endif