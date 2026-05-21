//DS3231_RTC.h

#ifndef DS3231_RTC_H
#define DS3231_RTC_H

#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>

class DS3231_RTC{
    public:
        DS3231_RTC();
        bool init(uint8_t sda_pin = 21, uint8_t scl_pin = 22);
        void update(); // читает текущее время из RTC и сохраняет в полях
        // Геттеры
        int getYear() const { return _year; }
        int getMonth() const { return _month; }
        int getDay() const { return _day; }
        int getHour() const { return _hour; }
        int getMinute() const { return _minute; }
        int getSecond() const { return _second; }
        String getDateTimeString() const; // например "2025-05-11 14:35:22"
        String getTimeString() const; // "14:35:22"
    private:
        RTC_DS3231 _rtc;
        int _year, _month, _day, _hour, _minute, _second;
        bool _initialized;
};

#endif