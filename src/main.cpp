
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// =====================================================
// Wi-Fi Access Point
// =====================================================

const char* AP_SSID = "MotorControl";
const char* AP_PASSWORD = "Silnik123";

// =====================================================
// TB6612FNG - NodeMCU ESP8266
// =====================================================

// D1 = GPIO5  -> PWMA (regulacja predkosci)
// D2 = GPIO4  -> AIN1 (kierunek)
// D5 = GPIO14 -> AIN2 (kierunek)
// D6 = GPIO12 -> STBY (wlaczenie sterownika)
/*
  przy sterowniku B32v4 podlaczone tylko 3 piny 
  D1 -> IN3
  D2 -> IN2
  D5 -> IN1
*/
constexpr uint8_t PIN_PWMA = D1;
constexpr uint8_t PIN_AIN1 = D2;
constexpr uint8_t PIN_AIN2 = D5;
constexpr uint8_t PIN_STBY = D6;

// =====================================================
// Ustawienia
// =====================================================

constexpr uint16_t PWM_MAX = 1023;
constexpr uint32_t HEARTBEAT_TIMEOUT_MS = 2500;

ESP8266WebServer server(80);

uint8_t speedPercent = 0;

// false = przod, true = tyl
bool reverseDirection = false;

uint32_t lastHeartbeat = 0;

// =====================================================
// Silnik
// =====================================================

void stopMotor() {
    speedPercent = 0;

    analogWrite(PIN_PWMA, 0);
    digitalWrite(PIN_STBY, LOW);
}

void setMotorSpeed(uint8_t percent) {
    percent = constrain(percent, 0, 100);

    if (percent == 0) {
        stopMotor();
        return;
    }

    // Najpierw ustaw kierunek
    digitalWrite(PIN_AIN1, reverseDirection ? LOW : HIGH);
    digitalWrite(PIN_AIN2, reverseDirection ? HIGH : LOW);

    // Wlacz sterownik
    digitalWrite(PIN_STBY, HIGH);

    // Ustaw predkosc PWM
    const uint16_t pwm =
        (static_cast<uint32_t>(percent) * PWM_MAX) / 100;

    analogWrite(PIN_PWMA, pwm);

    speedPercent = percent;
}

// =====================================================
// Strona WWW
// =====================================================

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pl">
<head>
<meta charset="UTF-8">

<meta name="viewport"
      content="width=device-width, initial-scale=1">

<title>Sterowanie silnikiem DC</title>

<style>
    body {
        font-family: Arial, sans-serif;
        max-width: 460px;
        margin: 30px auto;
        padding: 0 18px;
        background: #f1f3f5;
        color: #20242a;
    }

    .panel {
        background: white;
        border-radius: 16px;
        padding: 24px;
        box-shadow: 0 4px 18px #0001;
    }

    h1 {
        font-size: 24px;
        text-align: center;
    }

    .direction-buttons {
        display: flex;
        gap: 10px;
        margin: 22px 0;
    }

    button {
        border: 0;
        border-radius: 10px;
        padding: 16px 8px;
        font-size: 17px;
        font-weight: bold;
        cursor: pointer;
    }

    .direction-buttons button {
        flex: 1;
        background: #2463eb;
        color: white;
    }

    #value {
        font-size: 38px;
        font-weight: bold;
        text-align: center;
    }

    input[type=range] {
        width: 100%;
        margin: 24px 0;
    }

    #stop {
        width: 100%;
        background: #d62828;
        color: white;
    }

    #status {
        text-align: center;
        color: #555;
        margin-top: 20px;
    }
</style>
</head>

<body>
<div class="panel">

    <h1>Sterowanie silnikiem DC</h1>

    <div class="direction-buttons">
        <button id="forward">▲ PRZOD</button>
        <button id="reverse">▼ TYL</button>
    </div>

    <p>Ustawiona predkosc:</p>

    <div id="value">0%</div>

    <input
        id="speed"
        type="range"
        min="0"
        max="100"
        value="0"
        aria-label="Predkosc silnika">

    <button id="stop">STOP</button>

    <p id="status">Silnik zatrzymany</p>

</div>

<script>
const slider = document.getElementById('speed');
const value = document.getElementById('value');
const status = document.getElementById('status');

let currentSpeed = 0;
let requestInProgress = false;

async function sendSpeed(percent) {
    try {
        const response = await fetch(
            '/speed?v=' + percent,
            { cache: 'no-store' }
        );

        if (!response.ok) {
            throw new Error('HTTP error');
        }

        currentSpeed = percent;

        status.textContent = percent === 0
            ? 'Silnik zatrzymany'
            : 'Silnik pracuje';

    } catch (e) {
        status.textContent =
            'Brak polaczenia z ESP8266';
    }
}

async function sendDirection(direction) {
    try {
        const response = await fetch(
            '/direction?d=' + direction,
            { cache: 'no-store' }
        );

        if (!response.ok) {
            throw new Error('HTTP error');
        }

        status.textContent =
            direction === 'forward'
                ? 'Kierunek: przod'
                : 'Kierunek: tyl';

    } catch (e) {
        status.textContent =
            'Brak polaczenia z ESP8266';
    }
}

// Regulacja predkosci
slider.addEventListener('input', () => {
    value.textContent = slider.value + '%';
    sendSpeed(Number(slider.value));
});

// Zmiana kierunku
document.getElementById('forward')
    .addEventListener('click', () => {
        sendDirection('forward');
    });

document.getElementById('reverse')
    .addEventListener('click', () => {
        sendDirection('reverse');
    });

// STOP
document.getElementById('stop')
    .addEventListener('click', () => {
        slider.value = 0;
        value.textContent = '0%';
        sendSpeed(0);
    });

// Podtrzymanie komunikacji
async function heartbeat() {
    if (currentSpeed === 0 || requestInProgress) {
        return;
    }

    requestInProgress = true;

    try {
        const response = await fetch(
            '/heartbeat',
            { cache: 'no-store' }
        );

        if (!response.ok) {
            throw new Error('HTTP error');
        }

    } catch (e) {
        status.textContent = 'Utracono polaczenie';

    } finally {
        requestInProgress = false;
    }
}

setInterval(heartbeat, 500);
</script>

</body>
</html>
)rawliteral";

// =====================================================
// Obsluga HTTP
// =====================================================

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

    String argument = server.arg("v");

    // Odrzuc niepoprawne dane
    if (argument.length() == 0) {
        server.send(400, "text/plain", "Invalid speed");
        return;
    }

    for (size_t i = 0; i < argument.length(); i++) {
        if (!isDigit(argument[i])) {
            server.send(400, "text/plain", "Invalid speed");
            return;
        }
    }

    const int requested = argument.toInt();

    if (requested < 0 || requested > 100) {
        server.send(400, "text/plain", "Invalid speed");
        return;
    }

    lastHeartbeat = millis();

    setMotorSpeed(
        static_cast<uint8_t>(requested)
    );

    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "text/plain", "OK");
}

void handleDirection() {
    if (!server.hasArg("d")) {
        server.send(400, "text/plain", "Missing direction");
        return;
    }

    String direction = server.arg("d");

    if (direction != "forward" &&
        direction != "reverse") {
        server.send(400, "text/plain", "Invalid direction");
        return;
    }

    // Zatrzymaj silnik przed zmiana kierunku
    uint8_t previousSpeed = speedPercent;

    stopMotor();

    delay(50);

    reverseDirection = (direction == "reverse");

    // Przywroc poprzednia predkosc
    if (previousSpeed > 0) {
        setMotorSpeed(previousSpeed);
    }

    lastHeartbeat = millis();

    server.send(200, "text/plain", "OK");
}

void handleHeartbeat() {
    if (speedPercent > 0) {
        lastHeartbeat = millis();
    }

    server.send(200, "text/plain", "OK");
}

// =====================================================
// Setup
// =====================================================

void setup() {
    Serial.begin(115200);
    delay(100);

    pinMode(PIN_PWMA, OUTPUT);
    pinMode(PIN_AIN1, OUTPUT);
    pinMode(PIN_AIN2, OUTPUT);
    pinMode(PIN_STBY, OUTPUT);

    // Bezpieczny stan poczatkowy
    digitalWrite(PIN_STBY, LOW);
    digitalWrite(PIN_AIN1, LOW);
    digitalWrite(PIN_AIN2, LOW);

    analogWriteRange(PWM_MAX);
    analogWriteFreq(1000);
    analogWrite(PIN_PWMA, 0);

    // Uruchom wlasna siec Wi-Fi
    WiFi.persistent(false);
    WiFi.mode(WIFI_AP);
    WiFi.setSleepMode(WIFI_NONE_SLEEP);

    const bool apStarted =
        WiFi.softAP(AP_SSID, AP_PASSWORD);

    if (!apStarted) {
        Serial.println("ERROR: Access Point failed");
        stopMotor();
        return;
    }

    Serial.println();
    Serial.println("ESP8266 Motor Controller");

    Serial.print("Wi-Fi: ");
    Serial.println(AP_SSID);

    Serial.print("Panel: http://");
    Serial.println(WiFi.softAPIP());

    // Zarejestruj adresy
    server.on("/", HTTP_GET, handleRoot);
    server.on("/speed", HTTP_GET, handleSpeed);
    server.on("/direction", HTTP_GET, handleDirection);
    server.on("/heartbeat", HTTP_GET, handleHeartbeat);

    server.onNotFound([]() {
        server.send(404, "text/plain", "Not found");
    });

    server.begin();

    Serial.println("HTTP server started");
}

// =====================================================
// Loop
// =====================================================

void loop() {
    server.handleClient();

    // Zatrzymanie przy utracie heartbeat
    if (speedPercent > 0 &&
        millis() - lastHeartbeat >
            HEARTBEAT_TIMEOUT_MS) {

        stopMotor();

        Serial.println(
            "Motor stopped: heartbeat timeout"
        );
    }
}
