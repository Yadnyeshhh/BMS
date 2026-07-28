#pragma once

#include "../database/DatabaseManager.hpp"
#include <string>

namespace utils {

class InputSanitizer {
public:
    static std::string sanitize(const database::DatabaseManager& db, const std::string& input);
};

} // namespace utils
