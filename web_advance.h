#ifndef WEB_ADVANCE_H
#define WEB_ADVANCE_H

#include <Arduino.h>

const char HTML_ADVANCE[] PROGMEM = R"rawliteral(
<!doctype html>
<html>
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>Dieu Khien Quat Nidec</title>
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
      .container { max-width: 420px; width: 100%; margin: auto; flex-direction: column; gap: 1.25rem; text-align: center; }
      #basic_container { display: none; }
      #advance_container { display: flex; }
      h1 { color: var(--primary-500); font-size: 24px; }
      .value-display { font-size: 2rem; font-weight: bold; }
      .unit { font-size: 18px; }
      button {
        background-color: var(--neutral-100); color: var(--text); padding: 1.25rem 1.5rem;
        font-size: 1rem; cursor: pointer; font-weight: bold; border-radius: 9999px;
        border: none; transition: all 0.2s ease; box-shadow: 0 4px 10px rgba(0, 0, 0, 0.25);
      }
      button[aria-checked='true'], button:active { transform: scale(0.97); box-shadow: inset 0 3px 8px rgba(0, 0, 0, 0.7); }
      button.btn-stop { background-color: var(--danger-500); color: white; }
      .btn-primary { background-color: var(--primary-500); color: white; }
      .btn-muted { background-color: var(--neutral-200); }
      .btn-clear { background-color: var(--danger-500); color: white; }
      .card { background: #0f172a; border: 1px solid var(--border); border-radius: 16px; padding: 1rem; margin-top: 1rem; }
      .log-box { background: #020617; color: #7bf5b8; font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, "Liberation Mono", "Courier New", monospace; font-size: 13px; padding: 0.75rem; border-radius: 10px; text-align: left; height: 170px; overflow-y: auto; border: 1px solid var(--border); margin-bottom: 0.75rem; white-space: pre-wrap; }
      .learn-status { font-weight: 600; color: #fde047; margin: 0.35rem 0 0.75rem; min-height: 1.2rem; }
      .ir-grid { display: grid; grid-template-columns: repeat(3, minmax(0, 1fr)); gap: 0.5rem; }
      h2 { margin-top: 0.2rem; margin-bottom: 0.5rem; color: #93c5fd; font-size: 1.05rem; }
    </style>
  </head>
  <body>
    <div id="advance_container" class="container">
      <h1>Bo Doi Toc Quat Nidec</h1>
      <div>
        <div class="value-display"><span id="hz_val">0</span> <span class="unit">Hz</span></div>
        <p style="font-size: 14px; color: #888">Toc do: <span id="rpm_val">0</span> RPM</p>
      </div>

      <input type="range" min="0" max="450" value="0" class="slider" id="hzSlider" oninput="setFanSpeed(this.value)" />

      <button class="btn-stop" onclick="setFanSpeed(0)">Tat quat</button>

      <section class="card" id="ir_learn_panel">
        <h2>Hoc lenh IR (RAW)</h2>
        <div id="learn-status" class="learn-status">Trang thai: cho bat dau...</div>
        <div id="ir_log" class="log-box">Dang cho nhan du lieu IR...</div>

        <div class="ir-grid">
          <button class="btn-primary" onclick="startLearn('POWER')">Hoc POWER</button>
          <button class="btn-primary" onclick="startLearn('SPEED')">Hoc SPEED</button>
          <button class="btn-primary" onclick="startLearn('SWING')">Hoc SWING</button>
          <button class="btn-muted" onclick="clearIrLog()">Xoa log</button>
          <button class="btn-muted" onclick="copyIrLog()">Copy log</button>
          <button class="btn-clear" onclick="clearIrLearn()">Clear mode</button>
        </div>
      </section>
    </div>

    <div style="margin-top: 1rem; display: flex; gap: 0.5rem; justify-content: center; align-items: center;">
      <a href="/"><button>Cơ bản</button></a>
      <a href="/ir"><button>Trang IR Raw</button></a>
    </div>

    <!-- Dùng chung file Javascript -->
    <script src="/main.js"></script>
    <script>
      let irLog = "";

      function startLearn(cmd) {
        const status = document.getElementById('learn-status');
        if (status) status.innerText = "Dang hoc nut: " + cmd + "...";
        fetch('/start_learn?cmd=' + encodeURIComponent(cmd));
      }

      function clearIrLog() {
        const box = document.getElementById('ir_log');
        if (box) {
          box.innerText = '';
          irLog = '';
          const status = document.getElementById('learn-status');
          if (status) status.innerText = "Da xoa log nhan.";
        }
      }

      function copyIrLog() {
        if (!irLog) {
          alert('Chua co du lieu IR de copy.');
          return;
        }
        if (navigator.clipboard && navigator.clipboard.writeText) {
          navigator.clipboard.writeText(irLog).then(() => {
            alert('Da copy ma IR!');
          }).catch(() => fallbackCopyIr());
        } else {
          fallbackCopyIr();
        }
      }

      function fallbackCopyIr() {
        const ta = document.createElement('textarea');
        ta.value = irLog;
        ta.style.position = 'fixed';
        ta.style.opacity = '0';
        document.body.appendChild(ta);
        ta.focus();
        ta.select();
        try {
          document.execCommand('copy');
          alert('Da copy ma IR!');
        } catch (err) {
          alert('Khong copy duoc, vui long thu lai.');
        }
        document.body.removeChild(ta);
      }

      function clearIrLearn() {
        fetch('/start_learn?cmd=' + encodeURIComponent(''));
        if (document.getElementById('learn-status')) {
          document.getElementById('learn-status').innerText = 'Da xoa mode hoc.';
        }
      }

      async function fetchIrData() {
        try {
          const res = await fetch('/get_ir_data');
          const data = await res.json();
          if (!data || !data.received) return;

          const msg = data.learn_msg ? ` | ${data.learn_msg}` : '';
          const entry = `[${data.protocol}] Hex: ${data.hex} | Bits: ${data.bits}${msg}\n`;
          irLog += entry;
          const box = document.getElementById('ir_log');
          if (box) {
            box.innerText += entry;
            box.scrollTop = box.scrollHeight;
          }
          const status = document.getElementById('learn-status');
          if (status && data.learn_msg) status.innerText = data.learn_msg;
        } catch (err) {
          const status = document.getElementById('learn-status');
          if (status) status.innerText = 'Loi doc du lieu IR.';
        }
      }

      setInterval(fetchIrData, 350);
    </script>
  </body>
</html>
)rawliteral";

#endif
