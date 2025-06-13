// api.js
const API_BASE = 'http://1.95.61.131:5555/api';

export async function get(url) {
  const res = await fetch(API_BASE + url, { method: 'GET' });
  return await res.json();
}

export async function post(url, data) {
  const res = await fetch(API_BASE + url, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(data)
  });
  return await res.json();
}
