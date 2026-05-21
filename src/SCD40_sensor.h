//SCD40_sensor.h

#ifndef SCD40_SENSOR_H
#define SCD40_SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <SensirionI2CScd4x.h>

class SCD40_sensor {
public:
    SCD40_sensor();
    bool init(uint8_t sda_pin = 21, uint8_t scl_pin = 22);
    bool read();
    
    // Геттеры полученных значений
    uint16_t getCO2() const { return _co2; }
    float getTemperature() const { return _temperature; }
    float getHumidity() const { return _humidity; }
    bool isDataReady() const { return _data_ready; }
    String getStatusString() const;

private:
    SensirionI2cScd4x _sensor;
    
    uint16_t _co2;
    float _temperature;
    float _humidity;
    bool _data_ready;
    unsigned long _last_read_ms;
};

#endif