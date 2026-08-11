// Mở modal
function openModal(modalId) {
  const modal = document.getElementById(modalId);
  if (!modal) return;

  modal.classList.add('is-open');
  document.body.style.overflow = 'hidden';

  document.addEventListener('keydown', handleEscClose);
}

// Hàm đóng Modal
function closeModal(modalId) {
  const modal = document.getElementById(modalId);
  if (!modal) return;

  modal.classList.remove('is-open');
  document.body.style.overflow = '';

  document.removeEventListener('keydown', handleEscClose);
}

// Đóng modal khi bấm phím ESC
function handleEscClose(event) {
  if (event.key === 'Escape') {
    const activeModal = document.querySelector('.modal-overlay.is-open');
    if (activeModal) {
      closeModal(activeModal.id);
    }
  }
}

// Render log lên UI
function renderIrLogs(logs) {
  const ul = document.getElementById('ir-log');
  if (!ul) return;

  if (!logs.length) {
    renderEmptyState(ul);
    return;
  }

  const fragment = document.createDocumentFragment();

  logs.forEach((item) => {
    const li = document.createElement('li');
    li.className = 'ir-log-item';

    // Safe formatting cho Address & Command
    const addr = Number.isInteger(item.address)
      ? `0x${item.address.toString(16).toUpperCase()}`
      : item.address || 'N/A';

    const cmd = Number.isInteger(item.command)
      ? `0x${item.command.toString(16).toUpperCase()}`
      : item.command || 'N/A';

    li.innerHTML = `
      <span><b>Thời gian:</b> ${escapeHtml(item.time)}</span><br />
      <span><b>Giao thức:</b> ${escapeHtml(item.protocol)}</span><br />
      <span><b>Địa chỉ:</b> ${escapeHtml(addr)}</span><br />
      <span><b>Lệnh:</b> ${escapeHtml(cmd)}</span><br />
      <span><b>Mã HEX đầy đủ:</b> ${escapeHtml(item.rawHex)}</span>
    `;

    fragment.appendChild(li);
  });

  ul.replaceChildren(fragment);
}

// Xóa log
function clearIrLog() {
  const ul = document.getElementById('ir-log');
  if (!ul) return;
  renderEmptyState(ul);
}

// Helper render Empty State
function renderEmptyState(ulElement) {
  ulElement.innerHTML = '<li class="ir-log-empty">Chưa có dữ liệu log</li>';
}

// Helper tránh XSS
function escapeHtml(str = '') {
  return String(str)
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;')
    .replace(/'/g, '&#039;');
}

// TODO remove
renderIrLogs([
  {
    time: '14:05:12',
    protocol: 'SYMPHONY',
    address: 0x4c,
    command: 0xd81,
    rawHex: '0xD81 (12 Bits)', // Nút Off
  },
  {
    time: '14:04:45',
    protocol: 'SYMPHONY',
    address: 0x4c,
    command: 0xd90,
    rawHex: '0xD90 (12 Bits)', // Nút On / Speed 1
  },
  {
    time: '14:03:20',
    protocol: 'SYMPHONY',
    address: 0x4c,
    command: 0xd82,
    rawHex: '0xD82 (12 Bits)', // Speed 2
  },
  {
    time: '14:02:10',
    protocol: 'SYMPHONY',
    address: 0x4c,
    command: 0xd84,
    rawHex: '0xD84 (12 Bits)', // Speed 3
  },
  {
    time: '14:01:05',
    protocol: 'SYMPHONY',
    address: 0x4c,
    command: 0xd88,
    rawHex: '0xD88 (12 Bits)', // Swing (Đảo gió)
  },
  {
    time: '13:59:39',
    protocol: 'SYMPHONY',
    address: 0x4c,
    command: 0xd90,
    rawHex: '0xD90 (12 Bits)',
  },
  {
    time: '13:58:15',
    protocol: 'NEC',
    address: 0x00ff,
    command: 0x4522,
    rawHex: '0x00FF4522 (32 Bits)', // Log mở rộng giao thức NEC
  },
  {
    time: '13:55:00',
    protocol: 'SYMPHONY',
    address: 0x4c,
    command: 0xd98,
    rawHex: '0xD98 (12 Bits)', // Timer Hẹn giờ
  },
  {
    time: '13:50:12',
    protocol: 'PANASONIC',
    address: 0x4004,
    command: 0x100bcb2,
    rawHex: '0x4004100BCB2 (48 Bits)',
  },
  {
    time: '13:45:30',
    protocol: 'SYMPHONY',
    address: 0x4c,
    command: 0xd81,
    rawHex: '0xD81 (12 Bits)',
  },
]);
