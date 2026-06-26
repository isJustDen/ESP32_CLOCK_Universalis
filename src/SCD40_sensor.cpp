//SCD40_sensor.cpp

#include "SCD40_sensor.h"

#define SCD40_I2C_ADDRESS 0x62

SCD40_sensor::SCD40_sensor() : _co2(0), _temperature(0.0), _humidity(0.0), _data_ready(false), _last_read_ms(0) {}

bool SCD40_sensor::init(uint8_t sda_pin, uint8_t scl_pin) {
    Wire.begin(sda_pin, scl_pin);
    _sensor.begin(Wire, SCD40_I2C_ADDRESS);
    
    uint16_t error;
    char errorMessage[256];
    
    // Останавливаем любые предыдущие измерения (на всякий случай)
    error = _sensor.stopPeriodicMeasurement();
    if (error) {
        Serial.print("SCD40 stopPeriodicMeasurement error: ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
        return false;
    }
    
    // Запускаем периодические измерения (1 раз в 5 секунд)
    error = _sensor.startPeriodicMeasurement();
    if (error) {
        Serial.print("SCD40 startPeriodicMeasurement error: ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
        return false;
    }
    
    Serial.println("SCD40 initialized successfully");
    return true;
}

bool SCD40_sensor::read() {
    // Датчик сам измеряет в фоне, мы только забираем результат.
    // Ограничим частоту опроса: не чаще 1 раза в 5 секунд.
    if (millis() - _last_read_ms < 5000) {
        return _data_ready; // возвращаем старые данные, пока не прошло 5 сек
    }
    
    uint16_t error;
    char errorMessage[256];
    uint16_t co2 = 0;
    float temperature = 0.0;
    float humidity = 0.0;
    
    unsigned long t0 = millis();
    error = _sensor.readMeasurement(co2, temperature, humidity);
    unsigned long dt = millis() - t0; 
    if (dt > 100) { 
        Serial.print("SCD40 readMeasurement SLOW: "); 
        Serial.print(dt); 
        Serial.println(" ms"); 
    }
    
    if (error) {
        // Ошибка чтения — датчик, возможно, ещё не готов (первые 5 секунд после старта)
        _data_ready = false;
        return false;
    }
    
    if (co2 != 0) {
        _co2 = co2;
        _temperature = temperature;
        _humidity = humidity;
        _data_ready = true;
        _last_read_ms = millis();
        return true;
    }
    
    _data_ready = false;
    return false;
}

String SCD40_sensor::getStatusString() const {
    if (!_data_ready) {
        return "SCD40: no data";
    }
    String result = "CO2: ";
    result += String(_co2);
    result += " ppm, T: ";
    result += String(_temperature, 1);
    result += "°C, H: ";
    result += String(_humidity, 1);
    result += "%";
    return result;
}