#include <iostream>
#include <string>
#include "../src/database/DatabaseManager.hpp"
#include "../src/utils/PasswordHasher.hpp"

int main() {
    database::DatabaseManager db;
    // Adjust connection parameters as needed
    if (!db.connect("localhost", "root", "", 3306)) {
        std::cerr << "Failed to connect to database" << std::endl;
        return 1;
    }
    std::string query = "SELECT account_id, password FROM accounts";
    if (!db.executeQuery(query)) {
        std::cerr << "Failed to fetch accounts" << std::endl;
        return 1;
    }
    MYSQL_RES* res = db.storeResult();
    if (!res) {
        std::cerr << "No result set" << std::endl;
        return 1;
    }
    MYSQL_ROW row;
    int migrated = 0;
    while ((row = mysql_fetch_row(res))) {
        std::string accountId = row[0];
        std::string plainPwd = row[1] ? row[1] : "";
        std::string hashed = utils::PasswordHasher::hash(plainPwd);
        std::string safeHashed = db.escapeString(hashed);
        std::string update = "UPDATE accounts SET password_hash = '" + safeHashed + "' WHERE account_id = " + accountId;
        db.executeQuery(update);
        ++migrated;
    }
    mysql_free_result(res);
    std::cout << "Migrated " << migrated << " accounts" << std::endl;
    return 0;
}
