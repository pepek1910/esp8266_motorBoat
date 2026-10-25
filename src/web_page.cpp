#include <Arduino.h>
#include "web_page.h"

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pl">
<head>
<meta charset="UTF-8">
<meta name="viewport"
      content="width=device-width, initial-scale=1">

<title>Sterowanie silnikiem DC i serwem</title>

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

    h2 {
        margin-top: 28px;
        font-size: 19px;
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

    .value {
        font-size: 34px;
        font-weight: bold;
        text-align: center;
        margin: 12px 0;
    }

    input[type=range] {
        width: 100%;
        margin: 16px 0;
    }

    #stop {
        width: 100%;
        margin-top: 12px;
        background: #d62828;
        color: white;
    }

    #status {
        text-align: center;
        color: #555;
        margin-top: 20px;
    }

    .description {
        color: #666;
        font-size: 14px;
        line-height: 1.4;
    }

    hr {
        border: 0;
        border-top: 1px solid #ddd;
        margin: 28px 0;
    }
</style>
</head>

<body>
<div class="panel">

    <h1>Sterowanie</h1>

    <h2>Silnik DC</h2>

    <p class="description">
        Kierunek obrotów i prędkość silnika.
    </p>

    <div class="direction-buttons">
        <button id="forward">▲ PRZÓD</button>
        <button id="reverse">▼ TYŁ</button>
    </div>

    <p>Prędkość silnika:</p>
    <div class="value" id="speedValue">0%</div>

    <input
        id="speed"
        type="range"
        min="0"
        max="100"
        value="0"
        aria-label="Prędkość silnika">

    <button id="stop">STOP SILNIKA</button>

    <hr>

    <h2>Pozycja serwa</h2>

    <p class="description">
        Ustaw kąt, w którym skierowany jest silnik.
    </p>

    <div class="value" id="angleValue">45°</div>

    <input
        id="angle"
        type="range"
        min="0"
        max="90"
        value="45"
        aria-label="Kąt serwa">

    <p class="description">
        Zakres sterowania: 0°–90°.
    </p>

    <p id="status">Oczekiwanie na połączenie</p>

</div>

<script>
const speedSlider = document.getElementById('speed');
const speedValue = document.getElementById('speedValue');

const angleSlider = document.getElementById('angle');
const angleValue = document.getElementById('angleValue');

const statusElement = document.getElementById('status');

let currentSpeed = 0;
let requestInProgress = false;
let angleRequestTimer = null;

async function request(url) {
    const response = await fetch(url, {
        cache: 'no-store'
    });

    if (!response.ok) {
        throw new Error('HTTP ' + response.status);
    }
}

async function sendSpeed(percent) {
    try {
        await request('/speed?v=' + percent);

        currentSpeed = percent;

        statusElement.textContent = percent === 0
            ? 'Silnik zatrzymany'
            : 'Silnik pracuje';

    } catch (e) {
        statusElement.textContent = 'Brak połączenia z ESP8266';
    }
}

async function sendDirection(direction) {
    try {
        await request('/direction?d=' + direction);

        statusElement.textContent =
            direction === 'forward'
                ? 'Silnik DC: kierunek przód'
                : 'Silnik DC: kierunek tył';

    } catch (e) {
        statusElement.textContent = 'Brak połączenia z ESP8266';
    }
}

async function sendServoAngle(angle) {
    try {
        await request('/servo?angle=' + angle);
        statusElement.textContent = 'Ustawiono kąt serwa: ' + angle + '°';
    } catch (e) {
        statusElement.textContent = 'Błąd ustawiania serwa';
    }
}

// Prędkość silnika DC
speedSlider.addEventListener('input', () => {
    speedValue.textContent = speedSlider.value + '%';
    sendSpeed(Number(speedSlider.value));
});

// Kierunek obrotów silnika DC
document.getElementById('forward').addEventListener('click', () => {
    sendDirection('forward');
});

document.getElementById('reverse').addEventListener('click', () => {
    sendDirection('reverse');
});

// STOP dotyczy silnika DC, nie zmienia pozycji serwa
document.getElementById('stop').addEventListener('click', () => {
    speedSlider.value = 0;
    speedValue.textContent = '0%';
    sendSpeed(0);
});

// Kąt serwa — wysyłaj po krótkiej przerwie w przesuwaniu suwaka
angleSlider.addEventListener('input', () => {
    angleValue.textContent = angleSlider.value + '°';

    if (angleRequestTimer !== null) {
        clearTimeout(angleRequestTimer);
    }

    angleRequestTimer = setTimeout(() => {
        sendServoAngle(Number(angleSlider.value));
    }, 120);
});

// Podtrzymanie połączenia, gdy silnik DC pracuje
async function heartbeat() {
    if (currentSpeed === 0 || requestInProgress) {
        return;
    }

    requestInProgress = true;

    try {
        await request('/heartbeat');
    } catch (e) {
        statusElement.textContent = 'Utracono połączenie';
    } finally {
        requestInProgress = false;
    }
}

setInterval(heartbeat, 500);

request('/')
    .then(() => {
        statusElement.textContent = 'Połączono z ESP8266';
    })
    .catch(() => {
        statusElement.textContent = 'Brak połączenia z ESP8266';
    });
</script>

</body>
</html>
)rawliteral";