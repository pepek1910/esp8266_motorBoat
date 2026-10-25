#include <Arduino.h>
#include <Servo.h>

#include "motor_config.h"
#include "servo_controller.h"

namespace {
    Servo servo;
    uint8_t currentAngle = motor_config::SERVO_INITIAL_ANGLE;
}

namespace servo_controller {

void begin() {
    servo.attach(motor_config::PIN_SERVO);

    currentAngle = motor_config::SERVO_INITIAL_ANGLE;
    servo.write(currentAngle);

    delay(300);
}

void setAngle(uint8_t angle) {
    if (angle < motor_config::SERVO_MIN_ANGLE) {
        angle = motor_config::SERVO_MIN_ANGLE;
    }

    if (angle > motor_config::SERVO_MAX_ANGLE) {
        angle = motor_config::SERVO_MAX_ANGLE;
    }

    currentAngle = angle;
    servo.write(currentAngle);
}

uint8_t getAngle() {
    return currentAngle;
}

}