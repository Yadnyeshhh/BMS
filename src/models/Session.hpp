#pragma once

#include <string>

namespace models {

struct Session {
    bool isLoggedIn = false;
    int accountId = -1;
    std::string accountName = "";

    void login(int id, const std::string& name) {
        isLoggedIn = true;
        accountId = id;
        accountName = name;
    }

    void logout() {
        isLoggedIn = false;
        accountId = -1;
        accountName = "";
    }
};

} // namespace models
