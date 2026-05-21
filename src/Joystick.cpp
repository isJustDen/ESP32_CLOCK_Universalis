//Joystick.cpp

#include "Joystick.h"

Joystick::Joystick() : _pinX(0), _pinY(0), _pinSW(0), _xVal(0), _yVal(0), _pressed(false), _lastPressed(false), _justPressed(false), _direction(0), _deadzone(200) {} 

void Joystick::init(uint8_t pinX, uint8_t pinY, uint8_t pinSW) {
    _pinX = pinX;
    _pinY = pinY;
    _pinSW = pinSW;
    pinMode(_pinSW, INPUT_PULLUP); // подтяжка к 3.3V, нажатие замыкает на GND
}

void Joystick::update() {
    _xVal = analogRead(_pinX);
    _yVal = analogRead(_pinY);
    // Центр обычно около 2048, но может быть смещён. Определяем отклонение.
    int dx = _xVal - 2048;
    int dy = _yVal - 2048;

    // Определяем направление по наибольшему отклонению (крест-накрест)
    _direction = 0;
    if (abs(dx) > _deadzone || abs(dy)> _deadzone) {
        if(abs(dy) > abs(dx)){
            // вертикаль
            if (dy< -_deadzone) _direction = 1;  // вверх
            else if (dy>_deadzone) _direction = 2; // вниз
        } else {
            // горизонталь
            if (dx<-_deadzone) _direction = 3; // влево
            else if (dx>_deadzone) _direction = 4; // вправо
        }
    }

    // Обработка кнопки с антидребезгом
    bool reading = (digitalRead(_pinSW) == LOW); // LOW = нажато (из-за INPUT_PULLUP)
    // Простой антидребезг: запоминаем состояние с задержкой (лучше использовать таймер, но для простоты)
    delay(5); // маленькая задержка (не блокирующая, но здесь можно, так как update будет вызываться часто)
    if (reading == (digitalRead(_pinSW) == LOW)) {
        _pressed = reading;
    }
    _justPressed = (_pressed && !_lastPressed);
    _lastPressed = _pressed;
}