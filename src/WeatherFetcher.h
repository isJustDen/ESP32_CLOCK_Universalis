//WeatherFetcher.h
#ifndef WEATHER_FETCHER_H
#define WEATHER_FETCHER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

class WeatherFetcher {
    public:
        WeatherFetcher();
        void init(String city, String apiKey);    // Передаём название города (например, "Moscow") и API ключ
        void update();
        bool isDataFresh() const;    // Возвращает true, если есть свежие данные (менее 30 минут)
        
        // Геттеры для отображения
        String getCity() const { return _city; }
        float getTemp() const { return _temp; }
        float getFeelsLike() const { return _feelsLike; }
        int getHumidity() const { return _humidity; }
        float getPressure_hPa() const { return _pressure; }
        float getWindSpeed() const { return _windSpeed; }
        int getWindDeg() const { return _winDeg; }
        int getClouds() const { return _clouds; }
        int getSunset() const { return _sunset; }
        int getSunrise() const { return _sunrise; }
        int getTimezoneOffset () const { return _timezoneOffset; }
        String getDescription() const { return _description; }
        String getIconCode() const { return _iconCode; }
        String getLastError() const { return _lastError; }
        unsigned long getLastUpdateTime() const { return _lastUpdateTime; }

    private:
        bool _fetchData(); // выполняет HTTP запрос и парсинг
        void _parseJson(const String& json);

        String _city;
        String _apiKey;
        String _lastError;

        float _temp;
        float _feelsLike;
        int _humidity;
        float _pressure;
        float _windSpeed;
        int _winDeg;
        int _clouds;
        int _sunrise; 
        int _sunset;
        int _timezoneOffset;

        String _description;
        String _iconCode;
        
        unsigned long _lastUpdateTime;
        bool _dataValid;
        const unsigned long _updateInterval = 30 * 60 * 1000UL; // 30 минут
};

#endif