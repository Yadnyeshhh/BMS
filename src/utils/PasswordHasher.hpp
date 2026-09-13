#pragma once

#include <string>
#include <sstream>

namespace utils {

class PasswordHasher {
public:
    // Simple placeholder hash function. Replace with a proper bcrypt/argon2 implementation.
    static std::string hash(const std::string& password) {
        size_t hashed = std::hash<std::string>{}(password);
        std::ostringstream oss;
        oss << std::hex << hashed;
        return oss.str();
    }

    static bool verify(const std::string& password, const std::string& hashed) {
        return hash(password) == hashed;
    }
};

} // namespace utils
