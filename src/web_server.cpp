#include <Arduino.h>
#include <ESP8266WebServer.h>

#include "web_server.h"
#include "motor_controller.h"
#include "web_page.h"

namespace {
    ESP8266WebServer server(80);

    void handleRoot() {
        server.send_P(
            200,
            "text/html; charset=utf-8",
            INDEX_HTML
        );
    }

    void handleSpeed() {
        if (!server.hasArg("v")) {
            server.send(400, "text/plain", "Missing v");
            return;
        }

        const String argument = server.arg("v");

        if (argument.length() == 0) {
            server.send(400, "text/plain", "Invalid speed");
            return;
        }

        for (size_t i = 0; i < argument.length(); ++i) {
            if (!isDigit(argument[i])) {
                server.send(400, "text/plain", "Invalid speed");
                return;
            }
        }

        const int requested = argument.toInt();

        if (requested > 100) {
            server.send(400, "text/plain", "Invalid speed");
            return;
        }

        motor::refreshHeartbeat();
        motor::setSpeed(static_cast<uint8_t>(requested));

        server.sendHeader("Cache-Control", "no-store");
        server.send(200, "text/plain", "OK");
    }

    void handleDirection() {
        if (!server.hasArg("d")) {
            server.send(400, "text/plain", "Missing direction");
            return;
        }

        const String direction = server.arg("d");

        if (direction != "forward" && direction != "reverse") {
            server.send(400, "text/plain", "Invalid direction");
            return;
        }

        motor::setDirection(direction == "reverse");
        motor::refreshHeartbeat();

        server.send(200, "text/plain", "OK");
    }

    void handleHeartbeat() {
        if (motor::speedPercent() > 0) {
            motor::refreshHeartbeat();
        }

        server.send(200, "text/plain", "OK");
    }

    void handleNotFound() {
        server.send(404, "text/plain", "Not found");
    }
}

namespace web_server {

void begin() {
    server.on("/", HTTP_GET, handleRoot);
    server.on("/speed", HTTP_GET, handleSpeed);
    server.on("/direction", HTTP_GET, handleDirection);
    server.on("/heartbeat", HTTP_GET, handleHeartbeat);
    server.onNotFound(handleNotFound);

    server.begin();
}

void handleClient() {
    server.handleClient();
}

}