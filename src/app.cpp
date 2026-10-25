#include "app.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "motor_config.h"
#include "motor_controller.h"
#include "web_server.h"

namespace app {
void begin() {
    Serial.begin(115200);
    delay(100);

    motor::begin();

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

    Serial.println();
    Serial.println("ESP8266 Motor Controller");

    Serial.print("Wi-Fi: ");
    Serial.println(motor_config::AP_SSID);

    Serial.print("Panel: http://");
    Serial.println(WiFi.softAPIP());

    web_server::begin();

    Serial.println("HTTP server started");
}

void loop() {
    web_server::handleClient();

    if (motor::heartbeatTimedOut()) {
        motor::stop();
        Serial.println("Motor stopped: heartbeat timeout");
    }
}
}
