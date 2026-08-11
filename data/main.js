// Check var số Hz
function normalizeHz(val) {
  val = parseInt(val);

  if (Number.isNaN(val)) return 0;

  if (val > 0 && val < 50) return 50;

  return val;
}

// Lấy thông tin theo tốc độ. NOTE: Đổi số ở trên UI đơn giản phải kiểm tra if
function getSpeedInfo(val) {
  if (val === 0) {
    return {
      label: 'Tắt',
      button: document.querySelector('#basic_container [onclick*="0"]'),
    };
  }

  if (val <= 150) {
    return {
      label: 'Nhỏ',
      button: document.querySelector('#basic_container [onclick*="50"]'),
    };
  }

  if (val <= 350) {
    return {
      label: 'Vừa',
      button: document.querySelector('#basic_container [onclick*="250"]'),
    };
  }

  return {
    label: 'Lớn',
    button: document.querySelector('#basic_container [onclick*="450"]'),
  };
}

// Đồng bộ UI
function syncDisplay() {
  updateFanSpeedDisplay();
  updateSwingDisplay();
}

function updateFanSpeedDisplay(speed) {
  const normalizeSpeed = normalizeHz(speed ?? fanStatus.clk_hz);
  if (isAdvanceUI()) {
    updateAdvanceUI(normalizeSpeed);
    return;
  }

  updateBasicUI(normalizeSpeed);
}

function updateSwingDisplay(swing) {
  const button = document.getElementById('toggle_swing_btn');
  if (!button) return;
  const activeSwing = swing ?? !!fanStatus.swing;
  button.disabled = !fanStatus.power;
  button.setAttribute('aria-checked', String(activeSwing));
}

function isAdvanceUI() {
  const path = window.location.pathname.toLowerCase();
  const isAdvancePath = path === '/advance';

  const hasAdvanceContainer = !!document.getElementById('advance_container');

  return isAdvancePath || hasAdvanceContainer;
}

function updateBasicUI(speed) {
  const label = getSpeedInfo(speed).label;
  const readable = `Tốc độ quạt: ${label}`;
  document.getElementById('speed_label').innerText = label;
  document.getElementById('sr_label').innerText = readable;

  const buttons = document.querySelectorAll('#basic_container .speed-group button');
  const activeButton = getSpeedInfo(speed).button;

  buttons.forEach((btn) => {
    btn.setAttribute('aria-checked', btn === activeButton);
  });
}

function updateAdvanceUI(speed) {
  document.getElementById('hzSlider').value = speed;
  document.getElementById('hz_val').innerText = speed;
  document.getElementById('rpm_val').innerText = speed * 4;
}

// Gán tốc độ
function setFanSpeed(speed) {
  speed = normalizeHz(speed);

  if (pendingFanSpeed === speed) return;
  if (pendingFanSpeed === null && normalizeHz(fanStatus.clk_hz) === speed) return;

  sendFanSpeed(speed);
  updateFanSpeedDisplay(speed);
}

// Gửi tốc độ lên mạch
let sendFanSpeedTimeout;
let sendSwingTimeout;
let controlAbortController = null;
let controlRequestSequence = 0;
let pendingFanSpeed = null;
let pendingSwingState = null;
const STATUS_REFRESH_INTERVAL_MS = 1000;

async function readErrorResponse(response) {
  const message = await response.text();
  return message || `HTTP ${response.status}`;
}

async function refreshFanStatus() {
  if (statusRequestPromise) {
    await statusRequestPromise;
  }

  await getFanStatus();
}

function sendFanSpeed(speed) {
  clearTimeout(sendFanSpeedTimeout);
  clearTimeout(sendSwingTimeout);

  stopPolling();

  if (controlAbortController) {
    controlAbortController.abort();
  }
  controlAbortController = new AbortController();
  const controller = controlAbortController;
  const requestSequence = ++controlRequestSequence;
  pendingFanSpeed = speed;
  pendingSwingState = null;

  sendFanSpeedTimeout = setTimeout(async () => {
    try {
      const response = await fetch(`/set_real_clk?hz=${speed}`, {
        signal: controller.signal,
      });

      if (!response.ok) {
        throw new Error(await readErrorResponse(response));
      }
    } catch (e) {
      if (e.name === 'AbortError') return;
      console.error('Lỗi sendFanSpeed:', e);
    } finally {
      if (requestSequence === controlRequestSequence) {
        await refreshFanStatus();
        if (requestSequence === controlRequestSequence) {
          pendingFanSpeed = null;
          startPolling(STATUS_REFRESH_INTERVAL_MS);
        }
      }
    }
  }, 150);
}

// Đảo gió
function toggleSwing() {
  if (!fanStatus.power) return;

  const targetSwingState = !fanStatus.swing;
  if (pendingSwingState === targetSwingState) return;

  sendToggleSwing(targetSwingState);
  updateSwingDisplay(targetSwingState);
}

function sendToggleSwing(targetSwingState) {
  clearTimeout(sendSwingTimeout);
  clearTimeout(sendFanSpeedTimeout);

  stopPolling();

  if (controlAbortController) {
    controlAbortController.abort();
  }
  controlAbortController = new AbortController();
  const controller = controlAbortController;
  const requestSequence = ++controlRequestSequence;
  pendingFanSpeed = null;
  pendingSwingState = targetSwingState;

  sendSwingTimeout = setTimeout(async () => {
    try {
      const response = await fetch(`/set_swing?state=${targetSwingState ? 1 : 0}`, {
        signal: controller.signal,
      });

      if (!response.ok) {
        throw new Error(await readErrorResponse(response));
      }
    } catch (e) {
      if (e.name === 'AbortError') return;
      console.error('Lỗi toggleSwing:', e);
    } finally {
      if (requestSequence === controlRequestSequence) {
        await refreshFanStatus();
        if (requestSequence === controlRequestSequence) {
          pendingSwingState = null;
          startPolling(STATUS_REFRESH_INTERVAL_MS);
        }
      }
    }
  }, 150);
}

// Lấy status từ mạch
let statusAbortController = null;
let statusRequestPromise = null;
let liveStatusEnabled = false;
let liveStatusTimeout = null;
let fanStatus = {
  clk_hz: 0,
  power: 0,
  swing: 0,
};
async function getFanStatus() {
  if (statusRequestPromise) return statusRequestPromise;

  statusAbortController = new AbortController();
  const controller = statusAbortController;

  const request = (async () => {
    try {
      const response = await fetch('/status', { signal: controller.signal });

      if (!response.ok) {
        throw new Error(await readErrorResponse(response));
      }

      const data = await response.json();
      fanStatus = data;
      syncDisplay();
    } catch (e) {
      if (e.name !== 'AbortError') {
        console.error('Lỗi getFanStatus:', e);
      }
    } finally {
      if (statusRequestPromise === request) {
        statusRequestPromise = null;
        statusAbortController = null;

        if (liveStatusEnabled) {
          scheduleNextStatusUpdate();
        }
      }
    }
  })();

  statusRequestPromise = request;
  return request;
}

// Cập nhật status live, request trước xong mới gửi request kế tiếp
function scheduleNextStatusUpdate(delay = STATUS_REFRESH_INTERVAL_MS) {
  clearTimeout(liveStatusTimeout);
  liveStatusTimeout = setTimeout(getFanStatus, delay);
}

function startPolling(delay = 0) {
  stopPolling();
  liveStatusEnabled = true;
  scheduleNextStatusUpdate(delay);
}

function stopPolling() {
  liveStatusEnabled = false;
  clearTimeout(liveStatusTimeout);
  liveStatusTimeout = null;

  if (statusAbortController) {
    statusAbortController.abort();
  }
}

// Lấy thông tin khi load trang
window.onload = function () {
  startPolling();
};
