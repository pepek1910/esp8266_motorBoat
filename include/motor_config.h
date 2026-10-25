#pragma once

#include <Arduino.h>
#include <cstdint>

namespace motor_config {
    constexpr char AP_SSID[] = "MotorControl";
    constexpr char AP_PASSWORD[] = "Silnik123";

    constexpr uint8_t PIN_PWMA = D1;
    constexpr uint8_t PIN_AIN1 = D2;
    constexpr uint8_t PIN_AIN2 = D5;
    constexpr uint8_t PIN_STBY = D6;

    constexpr uint16_t PWM_MAX = 1023;
    constexpr uint16_t PWM_FREQUENCY_HZ = 1000;

    constexpr uint32_t HEARTBEAT_TIMEOUT_MS = 2500;
    constexpr uint32_t DIRECTION_CHANGE_DELAY_MS = 50;
}