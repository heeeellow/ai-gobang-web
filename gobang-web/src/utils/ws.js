let ws = null;
let handlers = [];
let wsQueue = []; // 新增队列

export function connectWS(token) {
  if (ws) ws.close();
  ws = new WebSocket(`ws://1.95.61.131:5566/ws?token=${token}`);
  ws.onmessage = e => {
    let msg;
    try { msg = JSON.parse(e.data); } catch { return; }
    handlers.forEach(fn => fn(msg));
  };
  ws.onclose = () => {
    handlers = [];
  };
  ws.onopen = () => {
    // 连接成功后自动把队列里所有消息都发出去
    wsQueue.forEach(msg => ws.send(msg));
    wsQueue = [];
  };
}

export function sendWSMsg(type, data = {}) {
  const user = JSON.parse(localStorage.getItem('user')) || {};
  if (user.id && !('user_id' in data)) data.user_id = user.id; // 自动附加 user_id
  const msgStr = JSON.stringify({ type, ...data });
  if (ws && ws.readyState === 1) {
    ws.send(msgStr);
  } else {
    wsQueue.push(msgStr); // 还没连接好就缓存，onopen时自动flush
  }
}

export function onWSMsg(fn) {
  handlers.push(fn);
}

export function closeWS() {
  if (ws) ws.close();
  ws = null;
  handlers = [];
  wsQueue = [];
}
