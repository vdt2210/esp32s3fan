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
function syncDisplay(val) {
  const label = getSpeedInfo(val).label;
  const readable = `Tốc độ quạt: ${label}`;
  document.getElementById('speed_label').innerText = label;
  document.getElementById('sr_label').innerText = readable;

  updateAdvanceUI(val);
  updateBasicUI(val);
}

function updateBasicUI(val) {
  const buttons = document.querySelectorAll('#basic_container button');
  const activeButton = getSpeedInfo(val).button;

  buttons.forEach((btn) => {
    btn.setAttribute('aria-checked', btn === activeButton);
  });
}

function updateAdvanceUI(val) {
  document.getElementById('hzSlider').value = val;
  document.getElementById('hz_val').innerText = val;
  document.getElementById('rpm_val').innerText = val * 4;
}

// Gán tốc độ
function setFanSpeed(speed) {
  speed = normalizeHz(speed);
  sendFanSpeed(speed);
  syncDisplay(speed);
}

// Gửi tốc độ lên mạch
let sendFanSpeedTimeout;
let sendAbortController = null;
function sendFanSpeed(speed) {
  clearTimeout(sendFanSpeedTimeout);

  stopPolling();

  if (sendAbortController) {
    sendAbortController.abort();
  }
  sendAbortController = new AbortController();

  sendFanSpeedTimeout = setTimeout(async () => {
    try {
      const res = await fetch(`/set_real_clk?hz=${speed}`, {
        signal: statusAbortController.signal,
      });
      const data = await res.text();
      console.log('CLK Update:', data);
    } catch (e) {
      if (e.name === 'AbortError') return;
      console.error('Lỗi sendFanSpeed:', e);
    } finally {
      retryCount = 0;
      startPolling();
    }
  }, 150);
}

// Lấy status từ mạch
let statusAbortController = null;
let retryCount = 0;
const MAX_RETRIES = 3;
async function getFanStatus() {
  if (retryCount >= MAX_RETRIES) {
    console.warn('Đã thất bại 3 lần liên tiếp, dừng polling!');
    clearInterval(pollFanStatusInterval);
    return;
  }

  if (statusAbortController) {
    statusAbortController.abort();
  }
  statusAbortController = new AbortController();

  try {
    const res = await fetch('/status', { signal: statusAbortController.signal });
    const data = await res.json();
    const hz = normalizeHz(data.clk_hz || 0);
    syncDisplay(hz);
    retryCount = 0;
    console.log('CLK status:', data);
  } catch (e) {
    if (e.name === 'AbortError') return;

    retryCount++;
    console.error(`Lỗi getFanStatus (${retryCount}/${MAX_RETRIES}):`, e);

    if (retryCount >= MAX_RETRIES) {
      console.error('Đã xảy ra lỗi 3 lần liên tiếp. Ngắt polling hoàn toàn.');
      clearInterval(pollFanStatusInterval);
    }
  }
}

// Poll status quạt
let pollFanStatusInterval;
function pollFanStatus() {
  clearInterval(pollFanStatusInterval);
  pollFanStatusInterval = setInterval(() => {
    getFanStatus();
  }, 1000);
}

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

// Lấy thông tin khi load trang
window.onload = function () {
  getFanStatus();
  startPolling();
};
