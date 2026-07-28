#pragma once

#include <mysql.h>
#include <string>

namespace database {

class DatabaseManager {
private:
    MYSQL* conn;

public:
    DatabaseManager();
    ~DatabaseManager();

    // Prevent copying
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool connect(const std::string& host, const std::string& user, const std::string& password, int port);
    bool initializeSchema();
    bool executeQuery(const std::string& query);
    MYSQL_RES* storeResult();
    std::string getLastError() const;
    my_ulonglong getInsertId() const;
    std::string escapeString(const std::string& input) const;
    MYSQL* getConnection() const;
};

} // namespace database
