//Joystick.h

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <Arduino.h>

class Joystick{
    public:
        Joystick();
        void init(uint8_t pinX, uint8_t pinY, uint8_t pinSW);
        void update();
        // Направления: 0=центр, 1=вверх, 2=вниз, 3=влево, 4=вправо
        int getDirection() const { return _direction; }
        bool isPressed() const { return _pressed; }
        bool justPressed() const {return _justPressed; } // фронт нажатия
    private:
        unsigned long _lastDebounceTime;
        uint8_t _pinX, _pinY, _pinSW;
        int _xVal, _yVal;
        bool _pressed;
        bool _lastPressed;
        bool _justPressed;
        bool _lastReading;
        bool _stableReading;
        bool _lastRawReading;
        int _direction;
        int _deadzone; // мёртвая зона, например 200

};

#endif