// #include <Arduino.h>
// #include <Wire.h>
// #include <SensirionI2CScd4x.h>

// SensirionI2cScd4x scd4x;

// void setup() {
//     Serial.begin(115200);
//     Wire.begin(21, 22);  // SDA = GPIO21, SCL = GPIO22
//     scd4x.begin(Wire, 0x62);
    
//     // Проверяем, есть ли датчик на шине
//     uint16_t error;
//     char errorMessage[256];
    
//     error = scd4x.stopPeriodicMeasurement();
//     if (error) {
//         Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
//         errorToString(error, errorMessage, 256);
//         Serial.println(errorMessage);
//     }
    
//     // Запускаем измерения с периодом 5 секунд
//     error = scd4x.startPeriodicMeasurement();
//     if (error) {
//         Serial.print("Error trying to execute startPeriodicMeasurement(): ");
//         errorToString(error, errorMessage, 256);
//         Serial.println(errorMessage);
//     }
    
//     Serial.println("SCD40 initialized. Waiting for first reading...");
// }

// void loop() {
//     uint16_t error;
//     char errorMessage[256];
//     uint16_t co2 = 0;
//     float temperature = 0.0;
//     float humidity = 0.0;
    
//     // Читаем данные
//     error = scd4x.readMeasurement(co2, temperature, humidity);
    
//     if (error) {
//         Serial.print("Error trying to execute readMeasurement(): ");
//         errorToString(error, errorMessage, 256);
//         Serial.println(errorMessage);
//     } else if (co2 == 0) {
//         Serial.println("Waiting for valid measurement...");
//     } else {
//         Serial.print("CO2: ");
//         Serial.print(co2);
//         Serial.print(" ppm, Temperature: ");
//         Serial.print(temperature);
//         Serial.print(" °C, Humidity: ");
//         Serial.print(humidity);
//         Serial.println(" %");
//     }
    
//     delay(5000); // используем только для теста
// }