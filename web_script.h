#ifndef WEB_SCRIPT_H
#define WEB_SCRIPT_H

#include <Arduino.h>

const char JS_MAIN[] PROGMEM = R"rawliteral(
const MODE = {
  advance: { label: 'Cho Hoàng', value: 'advance' },
  basic: { label: 'Cơ bản', value: 'basic' },
};

const MODE_SWITCH = {
  [MODE.advance.value]: MODE.basic,
  [MODE.basic.value]: MODE.advance,
};

let currentModeValue = MODE.basic.value;

function normalizeHz(val) {
  val = parseInt(val);
  if (Number.isNaN(val)) return 0;
  if (val > 0 && val < 50) return 50;
  return val;
}

function getSpeedInfo(val) {
  if (val === 0) {
    return { label: 'Tắt', button: document.querySelector('#basic_container [onclick*="0"]') };
  }
  if (val <= 150) {
    return { label: 'Nhỏ', button: document.querySelector('#basic_container [onclick*="50"]') };
  }
  if (val <= 350) {
    return { label: 'Vừa', button: document.querySelector('#basic_container [onclick*="250"]') };
  }
  return { label: 'Lớn', button: document.querySelector('#basic_container [onclick*="450"]') };
}

function syncDisplay(val) {
  const info = getSpeedInfo(val);
  const label = info ? info.label : 'Tắt';
  const speedLabel = document.getElementById('speed_label');
  if (speedLabel) speedLabel.innerText = label;
  
  const srLabel = document.getElementById('sr_label');
  if (srLabel) srLabel.innerText = `Tốc độ quạt: ${label}`;

  updateAdvanceUI(val);
  updateBasicUI(val);
}

function updateBasicUI(val) {
  const buttons = document.querySelectorAll('#basic_container button');
  const info = getSpeedInfo(val);
  const activeButton = info ? info.button : null;

  buttons.forEach((btn) => {
    btn.setAttribute('aria-checked', btn === activeButton);
  });
}

function updateAdvanceUI(val) {
  const slider = document.getElementById('hzSlider');
  if (slider) slider.value = val;
  
  const hzVal = document.getElementById('hz_val');
  if (hzVal) hzVal.innerText = val;
  
  const rpmVal = document.getElementById('rpm_val');
  if (rpmVal) rpmVal.innerText = val * 4;
}

function setFanSpeed(speed) {
  speed = normalizeHz(speed);
  sendFanSpeed(speed);
  syncDisplay(speed);
}

let sendFanSpeedTimeout;
let sendAbortController = null;
function sendFanSpeed(speed) {
  clearTimeout(sendFanSpeedTimeout);
  stopPolling();

  if (sendAbortController) sendAbortController.abort();
  sendAbortController = new AbortController();

  sendFanSpeedTimeout = setTimeout(async () => {
    try {
      const res = await fetch(`/set_real_clk?hz=${speed}`);
      const data = await res.text();
      console.log('CLK Update:', data);
    } catch (e) {
      console.error('Lỗi sendFanSpeed:', e);
    } finally {
      startPolling();
    }
  }, 150);
}

let statusAbortController = null;
async function getFanStatus() {
  try {
    const res = await fetch('/status');
    const data = await res.json();
    const hz = normalizeHz(data.clk_hz || 0);
    syncDisplay(hz);
  } catch (e) {
    if (e.name !== 'AbortError') {
      console.error('Lỗi getFanStatus:', e);
    }
  }
}

let pollFanStatusInterval;
function startPolling() {
  stopPolling();
  pollFanStatusInterval = setInterval(getFanStatus, 1000);
}

function stopPolling() {
  if (pollFanStatusInterval) {
    clearInterval(pollFanStatusInterval);
    pollFanStatusInterval = null;
  }
}

window.onload = function () {
  getFanStatus();
  startPolling();
};
)rawliteral";

#endif