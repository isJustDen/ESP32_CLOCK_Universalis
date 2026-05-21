// #include <Wire.h>
// #include <Arduino.h>
// #include <Adafruit_BMP280.h>


// void setup() {
//   Serial.begin(115200);
//   Wire.begin(21, 22);  // твои пины SDA=21, SCL=22
//   Serial.println("I2C Scanner");
//   Serial.println("----------------------------------");
// }

// void loop() {
//   byte error, address;
//   int nDevices = 0;
//   for(address = 1; address < 127; address++ ) {
//     Wire.beginTransmission(address);
//     error = Wire.endTransmission();
//     if (error == 0) {
//       Serial.print("Device found at address 0x");
//       if (address < 16) Serial.print("0");
//       Serial.println(address, HEX);
//       nDevices++;
//     }
//   }
//   if (nDevices == 0) Serial.println("No I2C devices found");
//   else Serial.println("----------------------------------");
//   delay(5000);
// }