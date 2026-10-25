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
    servo.attach(motor_config::PIN_SERVO, 400, 2200);
    currentAngle = motor_config::SERVO_INITIAL_ANGLE;
    servo.write(currentAngle);
    delay(300);
}

void setAngle(uint8_t angle) {
    if (angle < motor_config::SERVO_MIN_ANGLE)
        angle = motor_config::SERVO_MIN_ANGLE;

    if (angle > motor_config::SERVO_MAX_ANGLE)
        angle = motor_config::SERVO_MAX_ANGLE;

    currentAngle = angle;

    // Panel: 0–90°
    // Serwo: 0–72°, aby wartość 45° odpowiadała 36°
    const int servoAngle = map(angle, 0, 90, 0, 72);

    servo.write(servoAngle);
}

uint8_t getAngle() {
    return currentAngle;
}

}