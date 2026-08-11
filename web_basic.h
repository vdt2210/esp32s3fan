#ifndef WEB_BASIC_H
#define WEB_BASIC_H

#include <Arduino.h>

const char HTML_BASIC[] PROGMEM = R"rawliteral(
<!doctype html>
<html>
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>Điều Khiển Quạt Nidec</title>
    <style>
      :root {
        --bg: #020617;
        --neutral-100: #1e293b;
        --neutral-200: #334155;
        --text: #e2e8f0;
        --primary-400: #38bdf8;
        --primary-500: #0ea5e9;
        --primary-600: #0284c7;
        --danger-400: #f87171;
        --danger-500: #ef4444;
        --border: #1f2937;
        --focus: #38bdf8;
      }
      * { box-sizing: border-box; }
      html, body { margin: 0; padding: 0; height: 100%; width: 100%; }
      body {
        margin: 0; font-family: Arial, sans-serif; background: var(--bg);
        color: var(--text); display: flex; flex-direction: column;
        justify-content: center; align-items: center; height: 100vh; padding: 1.25rem;
      }
      .container { max-width: 400px; width: 100%; margin: auto; flex-direction: column; gap: 1.25rem; text-align: center; }
      #basic_container { display: flex; }
      #basic_container .speed-group { display: inherit; flex-direction: inherit; gap: inherit; margin: inherit; text-align: inherit; width: inherit; }
      #advance_container { display: none; }
      h1 { color: var(--primary-500); font-size: 24px; }
      .value-display { font-size: 2rem; font-weight: bold; }
      .unit { font-size: 18px; }
      .sr-only { position: absolute; left: -9999px; }
      button {
        background-color: var(--neutral-100); color: var(--text); padding: 1.25rem 1.5rem;
        font-size: 1rem; cursor: pointer; font-weight: bold; border-radius: 9999px;
        border: none; transition: all 0.2s ease; box-shadow: 0 4px 10px rgba(0, 0, 0, 0.25);
      }
      button[aria-checked='true'], button:active { transform: scale(0.97); box-shadow: inset 0 3px 8px rgba(0, 0, 0, 0.7); }
      button:disabled { opacity: 0.4; cursor: not-allowed; box-shadow: none; }
      button.btn-primary { background-color: var(--primary-500); color: white; }
      .btn-primary[aria-checked='true'] { background: var(--primary-600); }
      button.btn-stop { background-color: var(--danger-500); color: white; }
      .btn-stop[aria-checked='true'] { background: var(--danger-500); }
    </style>
  </head>
  <body>
    <span id="sr_label" class="sr-only" aria-live="polite" aria-atomic="true"></span>

    <div id="basic_container" class="container">
      <div role="radiogroup" aria-label="Điều khiển tốc độ quạt" class="speed-group">
        <button class="btn-stop" onclick="setFanSpeed(0)" role="radio" aria-checked="false" aria-label="Tắt quạt">Tắt</button>
        <button class="btn-primary" onclick="setFanSpeed(50)" role="radio" aria-checked="false" aria-label="Mức quạt nhỏ">Nhỏ</button>
        <button class="btn-primary" onclick="setFanSpeed(250)" role="radio" aria-checked="false" aria-label="Mức quạt vừa">Vừa</button>
        <button class="btn-primary" onclick="setFanSpeed(450)" role="radio" aria-checked="false" aria-label="Mức quạt lớn">Lớn</button>
      </div>
      <div>
        <span style="color: #888">Tốc độ:</span>
        <span id="speed_label" class="value-display">Tắt</span>
      </div>
    </div>

    <!-- Nối đường dẫn sang trang Nâng cao -->
    <a href="/advance">
      <button style="margin-top: 1rem;">Cho Hoàng</button>
    </a>

    <!-- Gọi file Script tách biệt -->
    <script src="/main.js"></script>
  </body>
</html>
)rawliteral";

#endif