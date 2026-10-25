
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// ---------- Wi-Fi Access Point ----------
const char* AP_SSID = "MotorControl";
const char* AP_PASSWORD = "Silnik123";

// ---------- TB6612FNG pins ----------
// NodeMCU D1 = GPIO5
// NodeMCU D2 = GPIO4
// NodeMCU D5 = GPIO14
// NodeMCU D6 = GPIO12
constexpr uint8_t PIN_PWMA = D1;
constexpr uint8_t PIN_AIN1 = D2;
constexpr uint8_t PIN_AIN2 = D5;
constexpr uint8_t PIN_STBY = D6;

// ---------- Motor settings ----------
constexpr uint16_t PWM_MAX = 1023;
constexpr uint32_t HEARTBEAT_TIMEOUT_MS = 2500;

ESP8266WebServer server(80);

uint8_t speedPercent = 0;
uint32_t lastHeartbeat = 0;

void stopMotor() {
    speedPercent = 0;
    analogWrite(PIN_PWMA, 0);
    digitalWrite(PIN_STBY, LOW);
}

void setMotorSpeed(uint8_t percent) {
    percent = constrain(percent, 0, 100);
    speedPercent = percent;

    if (percent == 0) {
        stopMotor();
        return;
    }

    // Fixed direction: AIN1 HIGH, AIN2 LOW.
    digitalWrite(PIN_AIN1, HIGH);
    digitalWrite(PIN_AIN2, LOW);

    digitalWrite(PIN_STBY, HIGH);

    const uint16_t pwm =
        (static_cast<uint32_t>(percent) * PWM_MAX) / 100;

    analogWrite(PIN_PWMA, pwm);
}

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pl">
<head>
<meta charset="UTF-8">
<meta name="viewport"
      content="width=device-width, initial-scale=1">
<title>Sterowanie silnikiem</title>
<style>
  body {
    font-family: system-ui, sans-serif;
    max-width: 460px;
    margin: 35px auto;
    padding: 0 20px;
    background: #f3f5f7;
    color: #20242a;
  }
  .panel {
    background: white;
    border-radius: 16px;
    padding: 24px;
    box-shadow: 0 4px 18px #0001;
  }
  h1 { font-size: 24px; }
  #value { font-size: 36px; font-weight: bold; }
  input[type=range] { width: 100%; margin: 24px 0; }
  button {
    width: 100%;
    border: 0;
    border-radius: 10px;
    padding: 16px;
    font-size: 18px;
    font-weight: bold;
    cursor: pointer;
  }
  #stop { background: #d62828; color: white; }
  #status { color: #555; margin-top: 18px; }
</style>
</head>
<body>
<div class="panel">
  <h1>Sterowanie silnikiem DC</h1>
  <p>Ustawiona prędkość:</p>
  <div id="value">0%</div>

  <input id="speed" type="range"
         min="0" max="100" value="0"
         aria-label="Prędkość silnika">

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
    const response = await fetch('/speed?v=' + percent);
    if (!response.ok) throw new Error('HTTP error');

    currentSpeed = percent;
    status.textContent = percent === 0
      ? 'Silnik zatrzymany'
      : 'Silnik pracuje';
  } catch (e) {
    status.textContent = 'Brak połączenia z ESP8266';
  }
}

slider.addEventListener('input', () => {
  value.textContent = slider.value + '%';
  sendSpeed(Number(slider.value));
});

document.getElementById('stop').addEventListener('click', () => {
  slider.value = 0;
  value.textContent = '0%';
  sendSpeed(0);
});

async function heartbeat() {
  if (currentSpeed === 0 || requestInProgress) return;

  requestInProgress = true;
  try {
    await fetch('/heartbeat', { cache: 'no-store' });
  } catch (e) {
    status.textContent = 'Utracono połączenie';
  } finally {
    requestInProgress = false;
  }
}

setInterval(heartbeat, 500);
</script>
</body>
</html>
)rawliteral";

void handleRoot() {
    server.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
}

void handleSpeed() {
    if (!server.hasArg("v")) {
        server.send(400, "text/plain", "Missing v");
        return;
    }

    const int requested = server.arg("v").toInt();

    if (requested < 0 || requested > 100) {
        server.send(400, "text/plain", "Invalid speed");
        return;
    }

    lastHeartbeat = millis();
    setMotorSpeed(static_cast<uint8_t>(requested));

    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "text/plain", "OK");
}

void handleHeartbeat() {
    if (speedPercent > 0) {
        lastHeartbeat = millis();
    }

    server.send(200, "text/plain", "OK");
}

void setup() {
    Serial.begin(115200);
    delay(100);

    pinMode(PIN_PWMA, OUTPUT);
    pinMode(PIN_AIN1, OUTPUT);
    pinMode(PIN_AIN2, OUTPUT);
    pinMode(PIN_STBY, OUTPUT);

    // Keep the motor disabled during startup.
    digitalWrite(PIN_STBY, LOW);
    digitalWrite(PIN_AIN1, LOW);
    digitalWrite(PIN_AIN2, LOW);
    analogWriteRange(PWM_MAX);
    analogWriteFreq(1000);
    analogWrite(PIN_PWMA, 0);

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

    Serial.print("Wi-Fi: ");
    Serial.println(AP_SSID);
    Serial.print("Panel: http://");
    Serial.println(WiFi.softAPIP());

    server.on("/", HTTP_GET, handleRoot);
    server.on("/speed", HTTP_GET, handleSpeed);
    server.on("/heartbeat", HTTP_GET, handleHeartbeat);

    server.onNotFound([]() {
        server.send(404, "text/plain", "Not found");
    });

    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();

    // Stop if the browser stops sending heartbeats.
    if (speedPercent > 0 &&
        millis() - lastHeartbeat > HEARTBEAT_TIMEOUT_MS) {
        stopMotor();
        Serial.println("Motor stopped: heartbeat timeout");
    }
}
