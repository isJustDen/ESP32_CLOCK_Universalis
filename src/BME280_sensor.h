// //BME280_sensor.h

// #ifndef BME280_SENSOR_H
// #define BME280_SENSOR_H

// #include <Arduino.h>
// #include <Wire.h>
// #include <Adafruit_BME280.h>

// class BME280_sensor {
//     public:
//         BME280_sensor();
//         bool init(uint8_t sda_pin = 21, uint8_t scl_pin = 22, uint8_t address = 0x76 );
//         bool read();
//         float getPressure_hPa() const { return _pressure_hPa; }
//         float getTemperature_C() const { return _temperature; }
//         float getHumidity() const {return _humidity;}
//         bool isOK() const { return _initialized && _data_ready; }
//     private:
//         Adafruit_BME280 _bme; // объект драйвера
//         float _pressure_hPa;
//         float _temperature;
//         float _humidity;
//         bool _initialized;
//         bool _data_ready;
//         unsigned long _last_read_ms;
// };

// #endif