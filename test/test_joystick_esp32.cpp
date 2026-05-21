// // Joystick_ESP32.ino
// #include <Arduino.h>
// #include <SPI.h>
// // Пример опроса аналогового джойстика KY-023 и его кнопки на ESP32.

// // Определяем пины
// const int PIN_JOY_X = 36;   // Ось X (VRx) - аналоговый пин
// const int PIN_JOY_Y = 39;   // Ось Y (VRy) - аналоговый пин
// const int PIN_JOY_SW = 34;  // Кнопка (SW) - цифровой пин

// // Переменные для хранения состояний
// int joyX_Value = 0;         // Сырое значение X (0-4095)
// int joyY_Value = 0;         // Сырое значение Y (0-4095)
// bool isSwPressed = false;   // Состояние кнопки (нажата/не нажата)
// bool lastSwState = HIGH;    // Для отслеживания фронта нажатия
// unsigned long lastDebounceTime = 0;
// const unsigned long debounceDelay = 50;

// void setup() {
//   Serial.begin(115200);
//   Serial.println("KY-023 Joystick Test Started!");

//   // Настройка пинов
//   pinMode(PIN_JOY_SW, INPUT_PULLUP); // Кнопка с подтяжкой к питанию

//   // Аналоговые пины не нужно настраивать через pinMode
// }

// void loop() {
//   // 1. Читаем аналоговые значения осей
//   // analogRead() возвращает значение от 0 до 4095 (12-битное АЦП)
//   joyX_Value = analogRead(PIN_JOY_X);
//   joyY_Value = analogRead(PIN_JOY_Y);

//   // 2. Обрабатываем кнопку с антидребезгом
//   bool currentSwState = digitalRead(PIN_JOY_SW);
  
//   if (currentSwState != lastSwState) {
//     lastDebounceTime = millis(); // Сброс таймера антидребезга
//   }
  
//   if ((millis() - lastDebounceTime) > debounceDelay) {
//     if (currentSwState != isSwPressed) {
//       isSwPressed = !isSwPressed;
//       if (!isSwPressed) { // Если кнопка была нажата и отпущена
//         Serial.println("Joystick Button Clicked!");
//       }
//     }
//   }
//   lastSwState = currentSwState;

//   // 3. Преобразуем сырые значения для удобства
//   // Это не обязательно, но полезно для дальнейшей логики
//   int mappedX = map(joyX_Value, 0, 4095, -100, 100);
//   int mappedY = map(joyY_Value, 0, 4095, -100, 100);
  
//   // Добавляем "мертвую зону" в центре
//   if (abs(mappedX) < 5) mappedX = 0;
//   if (abs(mappedY) < 5) mappedY = 0;

//   // 4. Вывод данных в монитор порта
//   Serial.print("X (raw): ");
//   Serial.print(joyX_Value);
//   Serial.print(" (mapped: ");
//   Serial.print(mappedX);
//   Serial.print(") | Y (raw): ");
//   Serial.print(joyY_Value);
//   Serial.print(" (mapped: ");
//   Serial.print(mappedY);
//   Serial.print(") | Button: ");
//   Serial.println(isSwPressed ? "Pressed" : "Released");

//   // Небольшая задержка для стабильности вывода
//   delay(100);
// }