#pragma once

#include <Arduino.h>

namespace servo_controller {
    void begin();
    void setAngle(uint8_t angle);
    uint8_t getAngle();
}