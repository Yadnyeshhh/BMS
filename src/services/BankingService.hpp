#pragma once

#include "../database/DatabaseManager.hpp"
#include "../models/Session.hpp"

namespace services {

class BankingService {
private:
    database::DatabaseManager& db;

    // Helper private method
    double getBalance(int accountId);

public:
    explicit BankingService(database::DatabaseManager& databaseMgr);

    void createAccount();
    void login(models::Session& session);
    void deposit(models::Session& session);
    void withdraw(models::Session& session);
    void transfer(models::Session& session);
    void checkBalance(const models::Session& session);
    void updateAccount(models::Session& session);
    void deleteAccount(models::Session& session);
    void logout(models::Session& session);
};

} // namespace services
