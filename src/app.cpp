#include "app.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "motor_config.h"
#include "motor_controller.h"
#include "servo_controller.h"
#include "web_server.h"

namespace {
    bool appReady = false;
    bool timeoutMessagePrinted = false;
}

namespace app {

void begin() {
    Serial.begin(115200);
    delay(100);

    motor::begin();
    servo_controller::begin();

    WiFi.persistent(false);
    WiFi.mode(WIFI_AP);
    WiFi.setSleepMode(WIFI_NONE_SLEEP);

    const bool apStarted =
        WiFi.softAP(motor_config::AP_SSID, motor_config::AP_PASSWORD);

    if (!apStarted) {
        Serial.println("ERROR: Access Point failed");
        motor::stop();
        return;
    }

    web_server::begin();
    appReady = true;

    Serial.println();
    Serial.println("ESP8266 Motor Controller");
    Serial.print("Wi-Fi: ");
    Serial.println(motor_config::AP_SSID);
    Serial.print("Panel: http://");
    Serial.println(WiFi.softAPIP());
    Serial.println("HTTP server started");
}

void loop() {
    if (!appReady) {
        return;
    }

    web_server::handleClient();

    if (motor::heartbeatTimedOut()) {
        motor::stop();

        if (!timeoutMessagePrinted) {
            Serial.println("Motor stopped: heartbeat timeout");
            timeoutMessagePrinted = true;
        }
    } else if (motor::speedPercent() > 0) {
        timeoutMessagePrinted = false;
    }
}

}