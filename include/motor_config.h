#pragma once

#include <Arduino.h>
#include <cstdint>

namespace motor_config {
    constexpr char AP_SSID[] = "MotorControl";
    constexpr char AP_PASSWORD[] = "Silnik123";

    // Sterownik silnika DC — TB6612FNG
    constexpr uint8_t PIN_PWMA = D1;
    constexpr uint8_t PIN_AIN1 = D2;
    constexpr uint8_t PIN_AIN2 = D5;
    constexpr uint8_t PIN_STBY = D7;

    // Serwo SG-92
    constexpr uint8_t PIN_SERVO = D6;
    constexpr uint8_t SERVO_MIN_ANGLE = 0;
    constexpr uint8_t SERVO_MAX_ANGLE = 90;
    constexpr uint8_t SERVO_INITIAL_ANGLE = 45;

    // PWM silnika DC
    constexpr uint16_t PWM_MAX = 1023;
    constexpr uint16_t PWM_FREQUENCY_HZ = 1000;

    // Zabezpieczenia
    constexpr uint32_t HEARTBEAT_TIMEOUT_MS = 2500;
    constexpr uint32_t DIRECTION_CHANGE_DELAY_MS = 50;
}