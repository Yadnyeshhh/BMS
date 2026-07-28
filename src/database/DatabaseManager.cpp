#include "DatabaseManager.hpp"
#include <iostream>

namespace database {

DatabaseManager::DatabaseManager() : conn(nullptr) {
    conn = mysql_init(NULL);
}

DatabaseManager::~DatabaseManager() {
    if (conn) {
        mysql_close(conn);
        conn = nullptr;
    }
}

bool DatabaseManager::connect(const std::string& host, const std::string& user, const std::string& password, int port) {
    if (!conn) {
        conn = mysql_init(NULL);
    }
    if (!conn) {
        return false;
    }
    // Connect to MySQL server (initially without selecting db)
    return mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(), NULL, port, NULL, 0) != nullptr;
}

bool DatabaseManager::initializeSchema() {
    if (!conn) return false;

    // Create database if not exists
    if (mysql_query(conn, "CREATE DATABASE IF NOT EXISTS bank_db")) {
        return false;
    }

    // Select database
    if (mysql_query(conn, "USE bank_db")) {
        return false;
    }

    // Create accounts table if it doesn't exist
    std::string createTableQuery =
        "CREATE TABLE IF NOT EXISTS accounts ("
        "    account_id INT AUTO_INCREMENT PRIMARY KEY,"
        "    name VARCHAR(100) NOT NULL,"
        "    password VARCHAR(100) NOT NULL,"
        "    balance DOUBLE NOT NULL DEFAULT 0.0"
        ")";

    if (mysql_query(conn, createTableQuery.c_str())) {
        return false;
    }

    return true;
}

bool DatabaseManager::executeQuery(const std::string& query) {
    if (!conn) return false;
    return mysql_query(conn, query.c_str()) == 0;
}

MYSQL_RES* DatabaseManager::storeResult() {
    if (!conn) return nullptr;
    return mysql_store_result(conn);
}

std::string DatabaseManager::getLastError() const {
    if (!conn) return "MySQL connection not initialized.";
    return mysql_error(conn);
}

my_ulonglong DatabaseManager::getInsertId() const {
    if (!conn) return 0;
    return mysql_insert_id(conn);
}

std::string DatabaseManager::escapeString(const std::string& input) const {
    if (!conn) return input;
    char* escaped = new char[input.length() * 2 + 1];
    unsigned long length = mysql_real_escape_string(conn, escaped, input.c_str(), input.length());
    std::string result(escaped, length);
    delete[] escaped;
    return result;
}

MYSQL* DatabaseManager::getConnection() const {
    return conn;
}

} // namespace database
