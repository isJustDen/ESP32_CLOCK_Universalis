// #include <Wire.h>
// #include <Adafruit_BME280.h>

// Adafruit_BME280 bme;  // по умолчанию использует Wire

// void setup() {
//   Serial.begin(115200);
//   Wire.begin(21, 22);
//   Wire.setClock(100000);
  
//   if (!bme.begin(0x76)) {
//     Serial.println("BME280 not found!");
//     while(1);
//   }
//   Serial.println("BME280 ready");
// }

// void loop() {
//   Serial.print("Temp: "); Serial.print(bme.readTemperature());
//   Serial.print(" °C, Pressure: "); Serial.print(bme.readPressure() / 100.0F);
//   Serial.print(" hPa, Humidity: "); Serial.println(bme.readHumidity());
//   delay(2000);
// }