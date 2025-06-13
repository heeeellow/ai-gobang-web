#pragma once
#include <string>

class UserConnection {
public:
    int fd;
    int user_id;
    int room_id;
    bool spectator;
    std::string username;

    UserConnection(int fd_, int user_id_=0, int room_id_=0, bool spectator_=false, const std::string& username_="") :
        fd(fd_), user_id(user_id_), room_id(room_id_), spectator(spectator_), username(username_) {}

    bool send_text(const std::string& msg) const;
};
