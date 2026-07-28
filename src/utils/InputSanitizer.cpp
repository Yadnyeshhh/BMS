#include "InputSanitizer.hpp"

namespace utils {

std::string InputSanitizer::sanitize(const database::DatabaseManager& db, const std::string& input) {
    return db.escapeString(input);
}

} // namespace utils
