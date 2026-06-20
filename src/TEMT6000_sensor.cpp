// //TEMT6000_sensor.cpp

// #include "TEMT6000_sensor.h"

// TEMT6000_sensor::TEMT6000_sensor() : _pin(0), _raw(0) {}

// void TEMT6000_sensor::init(uint8_t pin) {
//     _pin = pin;
//     analogSetPinAttenuation(_pin, ADC_11db);
//     pinMode(_pin, INPUT);
// }

// void TEMT6000_sensor::read() {
//     const int numReadings = 50;
//     long sum = 0;
//     for (int i = 0; i < numReadings; i++) {
//         sum += analogRead(_pin);
//         delayMicroseconds(50);
//     }
//     _raw = sum/numReadings;
// }

// float TEMT6000_sensor::getLux() const {
//     if (_raw < 10) return 0.0;

//     float voltage = (_raw/4095.0)*3.3; // Напряжение на входе ESP32
//     //  Закон Ома для делителя напряжения (R = 10 кОм)
//     float current__uA = (voltage / 10000.0)*1000000.0; // Ток через датчик в микроамперах
//     // Главная формула: 1 µA тока ~ 0.5 lx освещённости
//     float lux = current__uA*10.0;
//     return lux;
// }