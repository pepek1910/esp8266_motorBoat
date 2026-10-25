#include <Arduino.h>

#include "motor_controller.h"
#include "motor_config.h"

namespace {
    uint8_t currentSpeedPercent = 0;
    bool reverseDirection = false;
    uint32_t lastHeartbeat = 0;
}

namespace motor {

void begin() {
    pinMode(motor_config::PIN_PWMA, OUTPUT);
    pinMode(motor_config::PIN_AIN1, OUTPUT);
    pinMode(motor_config::PIN_AIN2, OUTPUT);
    pinMode(motor_config::PIN_STBY, OUTPUT);

    digitalWrite(motor_config::PIN_STBY, LOW);
    digitalWrite(motor_config::PIN_AIN1, LOW);
    digitalWrite(motor_config::PIN_AIN2, LOW);

    analogWriteRange(motor_config::PWM_MAX);
    analogWriteFreq(motor_config::PWM_FREQUENCY_HZ);
    analogWrite(motor_config::PIN_PWMA, 0);

    currentSpeedPercent = 0;
    reverseDirection = false;
    lastHeartbeat = millis();
}

void stop() {
    currentSpeedPercent = 0;

    analogWrite(motor_config::PIN_PWMA, 0);
    digitalWrite(motor_config::PIN_STBY, LOW);
}

void setSpeed(uint8_t percent) {
    if (percent > 100) {
        percent = 100;
    }

    if (percent == 0) {
        stop();
        return;
    }

    digitalWrite(
        motor_config::PIN_AIN1,
        reverseDirection ? LOW : HIGH
    );

    digitalWrite(
        motor_config::PIN_AIN2,
        reverseDirection ? HIGH : LOW
    );

    digitalWrite(motor_config::PIN_STBY, HIGH);

    const uint16_t pwm =
        (static_cast<uint32_t>(percent) * motor_config::PWM_MAX) / 100;

    analogWrite(motor_config::PIN_PWMA, pwm);
    currentSpeedPercent = percent;
}

void setDirection(bool reverse) {
    if (reverseDirection == reverse) {
        return;
    }

    const uint8_t previousSpeed = currentSpeedPercent;

    stop();
    delay(motor_config::DIRECTION_CHANGE_DELAY_MS);

    reverseDirection = reverse;

    if (previousSpeed > 0) {
        setSpeed(previousSpeed);
    }
}

uint8_t speedPercent() {
    return currentSpeedPercent;
}

void refreshHeartbeat() {
    lastHeartbeat = millis();
}

bool heartbeatTimedOut() {
    return currentSpeedPercent > 0 &&
        (millis() - lastHeartbeat >
         motor_config::HEARTBEAT_TIMEOUT_MS);
}

}