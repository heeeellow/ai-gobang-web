#include "UserConnection.h"
#include "network/ws_util.h"

bool UserConnection::send_text(const std::string& msg) const {
    return ws_send_text(fd, msg);
}
