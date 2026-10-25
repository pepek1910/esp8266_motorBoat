#pragma once

#include <Arduino.h>

namespace motor {
void begin();
void stop();
void setSpeed(uint8_t percent);
void setDirection(bool reverse);
uint8_t speedPercent();
void refreshHeartbeat();
bool heartbeatTimedOut();
}
