//Joystick.cpp

#include "Joystick.h"

Joystick::Joystick() : _pinX(0), _pinY(0), _pinSW(0), _xVal(0), _yVal(0), _pressed(false), _lastPressed(false), _justPressed(false), _direction(0), _deadzone(200) {} 

void Joystick::init(uint8_t pinX, uint8_t pinY, uint8_t pinSW) {
    _pinX = pinX;
    _pinY = pinY;
    _pinSW = pinSW;
    pinMode(_pinSW, INPUT_PULLUP); // подтяжка к 3.3V, нажатие замыкает на GND
    _lastDebounceTime = 0;
    _lastRawReading = HIGH;
    _stableReading = HIGH;
    _lastPressed = HIGH;
    _justPressed = false;
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

// ---- Антидребезг кнопки (неблокирующий) ----
    bool reading = (digitalRead(_pinSW) == LOW); // LOW = нажато

    // Если состояние изменилось, сбрасываем таймер
    if (reading != _lastRawReading) {
        _lastDebounceTime = millis();
    }

    // Если прошло достаточно времени (50 мс), принимаем состояние как стабильное
    if ((millis() - _lastDebounceTime) > 50) {
        if (reading != _stableReading) {
            // Состояние стабилизировалось и изменилось
            _stableReading = reading;
        }
    }

    _lastRawReading = reading;

    // ---- Генерация событий на основе стабильного состояния ----
    bool isPressedNow = (_stableReading == LOW); // нажато, если LOW
    _justPressed = (isPressedNow && !_lastPressed);
    _lastPressed = isPressedNow;
}