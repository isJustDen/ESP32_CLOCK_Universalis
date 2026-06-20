// //BME280_sensor.cpp

// #include "BME280_sensor.h"

// BME280_sensor::BME280_sensor() : _pressure_hPa(0), _temperature(0),_humidity(0), _initialized(false), _data_ready(false), _last_read_ms(0) {}

// bool BME280_sensor::init(uint8_t sda_pin, uint8_t scl_pin, uint8_t address){
//     Wire.begin(sda_pin, scl_pin);
//     // Передаём адрес, так как BMP280 может быть 0x76 или 0x77
//     if (!_bme.begin(address)){
//         Serial.println("BME280 not found!");
//         return false;
//     }
//     // Настройки: режим нормальный, oversampling x1, фильтр x2
//     _bme.setSampling(
//         Adafruit_BME280::MODE_NORMAL,
//         Adafruit_BME280::SAMPLING_X1, // температура
//         Adafruit_BME280::SAMPLING_X1, // давление
//         Adafruit_BME280::SAMPLING_X1, // Влажность
//         Adafruit_BME280::FILTER_X2,
//         Adafruit_BME280::STANDBY_MS_1000);
//     _initialized = true;
//     return true;
// }

// bool BME280_sensor::read() {
//     if (!_initialized) return false;
//     // Не чаще 1 раза в 2 секунды (BME280 обновляется медленнее)
//     if (millis() - _last_read_ms < 2000) {
//         return _data_ready;
//     }
//     _pressure_hPa = _bme.readPressure() / 100.0F;
//     _temperature = _bme.readTemperature();
//     _humidity = _bme.readHumidity();
//     _data_ready = true;
//     _last_read_ms = millis();
//     return true;
// }