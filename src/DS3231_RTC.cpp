//DS3231_RTC.cpp

#include "DS3231_RTC.h"

DS3231_RTC::DS3231_RTC() : _initialized(false), _year(0), _month(0), _day(0), _hour(0), _minute(0), _second(0) {}

bool DS3231_RTC::init(uint8_t sda_pin, uint8_t scl_pin) {
    Wire.begin(sda_pin, scl_pin);
    if (!_rtc.begin(&Wire)) {
        Serial.println("DS3231 not found");
        return false;
    }
    // Проверяем, не остановлен ли чип (например, батарейка села)
    if (_rtc.lostPower()){
        Serial.println("RTC lost power, setting default time (compile time)");
        // Устанавливаем время из времени компиляции (полезно при первом запуске)
        _rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    _initialized = true;
    update(); // сразу читаем время
    return true;
}

void DS3231_RTC ::update() {
    if (!_initialized) return;
    DateTime now = _rtc.now();
    _year = now.year();
    _month = now.month();
    _day = now.day();
    _hour = now.hour();
    _minute = now.minute();
    _second = now.second();
}

String DS3231_RTC::getDateTimeString() const {
    char buf[20];
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", _year, _month, _day, _hour, _minute, _second);
    return String(buf);
}

String DS3231_RTC::getTimeString() const {
    char buf[9];
    sprintf(buf, "%02d:%02d:%02d", _hour, _minute, _second);
    return String(buf);
}