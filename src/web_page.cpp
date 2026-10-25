#include "web_page.h"

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
