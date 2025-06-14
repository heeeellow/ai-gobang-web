#include "ws_util.h"
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <cstring>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <vector>
// base64 encode (using openssl)
std::string base64_encode(const unsigned char* data, size_t len) {
    BIO* bmem = BIO_new(BIO_s_mem());
    BIO* b64 = BIO_new(BIO_f_base64());
    b64 = BIO_push(b64, bmem);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(b64, data, len);
    BIO_flush(b64);
    BUF_MEM* bptr;
    BIO_get_mem_ptr(b64, &bptr);
    std::string ret(bptr->data, bptr->length);
    BIO_free_all(b64);
    return ret;
}

// 接收一行
std::string recv_line(int fd) {
    std::string line;
    char c;
    while (read(fd, &c, 1) == 1) {
        if (c == '\r') continue;
        if (c == '\n') break;
        line += c;
    }
    return line;
}

bool websocket_handshake(int fd) {
    std::string request;
    while (true) {
        std::string line = recv_line(fd);
        if (line.empty()) break;
        request += line + "\n";
    }
    std::string key;
    std::istringstream ss(request);
    std::string l;
    while (std::getline(ss, l)) {
        if (l.find("Sec-WebSocket-Key:") != std::string::npos) {
            key = l.substr(l.find(":")+1);
            while (!key.empty() && (key[0] == ' ' || key[0] == '\t')) key.erase(0,1);
            key.erase(key.find_last_not_of("\r\n")+1);
        }
    }
    if (key.empty()) return false;
    key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    unsigned char sha1res[SHA_DIGEST_LENGTH];
    SHA1((const unsigned char*)key.c_str(), key.size(), sha1res);
    std::string accept_key = base64_encode(sha1res, SHA_DIGEST_LENGTH);

    std::ostringstream resp;
    resp << "HTTP/1.1 101 Switching Protocols\r\n"
         << "Upgrade: websocket\r\n"
         << "Connection: Upgrade\r\n"
         << "Sec-WebSocket-Accept: " << accept_key << "\r\n\r\n";
    std::string s = resp.str();
    send(fd, s.data(), s.size(), 0);
    return true;
}

// 读取一个WebSocket帧文本消息
bool ws_recv_text(int fd, std::string& out) {
    unsigned char hdr[2];
    if (recv(fd, hdr, 2, MSG_WAITALL) != 2) return false;
    bool fin   = hdr[0] & 0x80;
    int opcode = hdr[0] & 0x0F;
    bool mask  = hdr[1] & 0x80;
    uint64_t len = hdr[1] & 0x7F;

    if (opcode == 8) return false;           // close 帧：告知上层正常关闭
    if (opcode == 9) {                       // ping ➜ pong
        ws_send_text(fd, "");                // 简易 pong
        return true;
    }
    if (opcode != 1) return true;            // 非文本帧直接忽略

    if (len == 126) { unsigned char ext[2];  recv(fd, ext, 2, MSG_WAITALL); len = (ext[0] << 8) | ext[1]; }
    else if (len == 127) {
        unsigned char ext[8]; recv(fd, ext, 8, MSG_WAITALL);
        len = 0; for (int i = 0; i < 8; ++i) len = (len << 8) | ext[i];
    }
    unsigned char mkey[4] = {};
    if (mask) recv(fd, mkey, 4, MSG_WAITALL);

    std::vector<unsigned char> payload(len);
    if (len) recv(fd, payload.data(), len, MSG_WAITALL);
    if (mask) for (uint64_t i = 0; i < len; ++i) payload[i] ^= mkey[i % 4];

    out.assign(reinterpret_cast<char*>(payload.data()), len);
    return true;                             // 对文本/控制帧均返回 true，交由上层决定
}


// 发送一个文本消息
bool ws_send_text(int fd, const std::string& text) {
    std::vector<unsigned char> frame;
    frame.push_back(0x81); // FIN + 文本帧
    size_t len = text.size();
    if (len < 126) frame.push_back(len);
    else if (len <= 0xFFFF) {
        frame.push_back(126);
        frame.push_back((len >> 8) & 0xFF);
        frame.push_back(len & 0xFF);
    } else {
        frame.push_back(127);
        for (int i = 7; i >= 0; --i)
            frame.push_back((len >> (8*i)) & 0xFF);
    }
    frame.insert(frame.end(), text.begin(), text.end());
    return send(fd, frame.data(), frame.size(), 0) == (int)frame.size();
}

