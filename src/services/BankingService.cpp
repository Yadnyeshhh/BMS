#include "BankingService.hpp"
#include "../utils/InputSanitizer.hpp"
#include <iostream>
#include <limits>
#include <iomanip>

using namespace std;

namespace services {

BankingService::BankingService(database::DatabaseManager& databaseMgr) : db(databaseMgr) {}

double BankingService::getBalance(int accountId) {
    string query = "SELECT balance FROM accounts WHERE account_id = " + to_string(accountId);
    if (!db.executeQuery(query)) {
        return -1.0;
    }
    
    MYSQL_RES* res = db.storeResult();
    if (!res) return -1.0;
    
    MYSQL_ROW row = mysql_fetch_row(res);
    double balance = -1.0;
    if (row) {
        balance = stod(row[0]);
    }
    mysql_free_result(res);
    return balance;
}

// 1. Account Creation
void BankingService::createAccount() {
    cout << "\n=== CREATE NEW ACCOUNT ===\n";
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear buffer
    
    string name;
    cout << "Enter Name: ";
    getline(cin, name);
    if (name.empty()) {
        cout << "[ERROR] Name cannot be empty.\n";
        return;
    }
    
    string password;
    cout << "Enter Password: ";
    getline(cin, password);
    if (password.empty()) {
        cout << "[ERROR] Password cannot be empty.\n";
        return;
    }
    
    string safeName = utils::InputSanitizer::sanitize(db, name);
    string safePassword = utils::InputSanitizer::sanitize(db, password);
    
    string query = "INSERT INTO accounts (name, password, balance) VALUES ('" + safeName + "', '" + safePassword + "', 0.0)";
    
    if (!db.executeQuery(query)) {
        cout << "[ERROR] Error creating account: " << db.getLastError() << "\n";
    } else {
        my_ulonglong newId = db.getInsertId();
        cout << "[SUCCESS] Account created successfully!\n";
        cout << "[INFO] Your Account ID is: " << newId << "\n";
        cout << "[WARNING] Please note down your Account ID. You will need it to login.\n";
    }
}

// 2. Login System
void BankingService::login(models::Session& session) {
    cout << "\n=== LOGIN ===\n";
    int accountId;
    cout << "Enter Account ID: ";
    if (!(cin >> accountId)) {
        cout << "[ERROR] Invalid input. Account ID must be a number.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }
    
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear buffer
    
    string password;
    cout << "Enter Password: ";
    getline(cin, password);
    
    string safePassword = utils::InputSanitizer::sanitize(db, password);
    
    string query = "SELECT name FROM accounts WHERE account_id = " + to_string(accountId) + " AND password = '" + safePassword + "'";
    
    if (!db.executeQuery(query)) {
        cout << "[ERROR] Database error during login: " << db.getLastError() << "\n";
        return;
    }
    
    MYSQL_RES* res = db.storeResult();
    if (!res) {
        cout << "[ERROR] Database error retrieving result: " << db.getLastError() << "\n";
        return;
    }
    
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row) {
        session.login(accountId, row[0]);
        cout << "\n[SUCCESS] Login successful! Welcome back, " << session.accountName << "!\n";
    } else {
        cout << "[ERROR] Invalid Account ID or Password.\n";
    }
    
    mysql_free_result(res);
}

// 3. Deposit Money
void BankingService::deposit(models::Session& session) {
    if (!session.isLoggedIn) {
        cout << "[ERROR] You must be logged in to perform this operation.\n";
        return;
    }
    
    cout << "\n=== DEPOSIT MONEY ===\n";
    double amount;
    cout << "Enter deposit amount: ";
    if (!(cin >> amount) || amount <= 0) {
        cout << "[ERROR] Invalid amount. Deposit amount must be a positive number.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }
    
    string query = "UPDATE accounts SET balance = balance + " + to_string(amount) + " WHERE account_id = " + to_string(session.accountId);
    
    if (!db.executeQuery(query)) {
        cout << "[ERROR] Deposit failed: " << db.getLastError() << "\n";
    } else {
        cout << "[SUCCESS] Deposit of $" << fixed << setprecision(2) << amount << " successful!\n";
        
        // Retrieve and print new balance
        double currentBalance = getBalance(session.accountId);
        if (currentBalance >= 0) {
            cout << "[BALANCE] New Balance: $" << fixed << setprecision(2) << currentBalance << "\n";
        }
    }
}

// 4. Withdraw Money
void BankingService::withdraw(models::Session& session) {
    if (!session.isLoggedIn) {
        cout << "[ERROR] You must be logged in to perform this operation.\n";
        return;
    }
    
    cout << "\n=== WITHDRAW MONEY ===\n";
    double amount;
    cout << "Enter withdrawal amount: ";
    if (!(cin >> amount) || amount <= 0) {
        cout << "[ERROR] Invalid amount. Withdrawal amount must be a positive number.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }
    
    double currentBalance = getBalance(session.accountId);
    if (currentBalance < 0) {
        cout << "[ERROR] Error retrieving balance.\n";
        return;
    }
    
    if (currentBalance < amount) {
        cout << "[ERROR] Insufficient balance! Current Balance: $" << fixed << setprecision(2) << currentBalance << "\n";
        return;
    }
    
    string query = "UPDATE accounts SET balance = balance - " + to_string(amount) + " WHERE account_id = " + to_string(session.accountId);
    
    if (!db.executeQuery(query)) {
        cout << "[ERROR] Withdrawal failed: " << db.getLastError() << "\n";
    } else {
        cout << "[SUCCESS] Withdrawal of $" << fixed << setprecision(2) << amount << " successful!\n";
        
        double newBalance = getBalance(session.accountId);
        if (newBalance >= 0) {
            cout << "[BALANCE] New Balance: $" << fixed << setprecision(2) << newBalance << "\n";
        }
    }
}

// 5. Transfer Money (TRANSACTION-BASED)
void BankingService::transfer(models::Session& session) {
    if (!session.isLoggedIn) {
        cout << "[ERROR] You must be logged in to perform this operation.\n";
        return;
    }
    
    cout << "\n=== TRANSFER MONEY ===\n";
    int receiverId;
    cout << "Enter receiver's Account ID: ";
    if (!(cin >> receiverId)) {
        cout << "[ERROR] Invalid Account ID.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }
    
    if (receiverId == session.accountId) {
        cout << "[ERROR] You cannot transfer money to yourself.\n";
        return;
    }
    
    double amount;
    cout << "Enter transfer amount: ";
    if (!(cin >> amount) || amount <= 0) {
        cout << "[ERROR] Invalid amount. Transfer amount must be a positive number.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }
    
    // Check if receiver exists
    string checkReceiverQuery = "SELECT name FROM accounts WHERE account_id = " + to_string(receiverId);
    if (!db.executeQuery(checkReceiverQuery)) {
        cout << "[ERROR] Database error checking receiver: " << db.getLastError() << "\n";
        return;
    }
    
    MYSQL_RES* res = db.storeResult();
    if (!res) {
        cout << "[ERROR] Database error retrieving receiver info.\n";
        return;
    }
    
    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) {
        cout << "[ERROR] Receiver Account ID not found.\n";
        mysql_free_result(res);
        return;
    }
    string receiverName = row[0];
    mysql_free_result(res);
    
    // Confirm transfer
    cout << "Are you sure you want to transfer $" << fixed << setprecision(2) << amount 
         << " to " << receiverName << " (ID: " << receiverId << ")? (y/n): ";
    char confirm;
    cin >> confirm;
    if (confirm != 'y' && confirm != 'Y') {
        cout << "[ERROR] Transfer cancelled.\n";
        return;
    }
    
    // Start SQL Transaction
    if (!db.executeQuery("START TRANSACTION")) {
        cout << "[ERROR] Failed to start transaction: " << db.getLastError() << "\n";
        return;
    }
    
    // Check sender balance inside transaction for safety
    double currentBalance = getBalance(session.accountId);
    if (currentBalance < 0) {
        cout << "[ERROR] Failed to retrieve current balance. Transaction rolled back.\n";
        db.executeQuery("ROLLBACK");
        return;
    }
    
    if (currentBalance < amount) {
        cout << "[ERROR] Insufficient balance! Transaction rolled back.\n";
        db.executeQuery("ROLLBACK");
        return;
    }
    
    // Deduct from sender
    string deductQuery = "UPDATE accounts SET balance = balance - " + to_string(amount) + " WHERE account_id = " + to_string(session.accountId);
    if (!db.executeQuery(deductQuery)) {
        cout << "[ERROR] Deduction failed: " << db.getLastError() << ". Transaction rolled back.\n";
        db.executeQuery("ROLLBACK");
        return;
    }
    
    // Add to receiver
    string addQuery = "UPDATE accounts SET balance = balance + " + to_string(amount) + " WHERE account_id = " + to_string(receiverId);
    if (!db.executeQuery(addQuery)) {
        cout << "[ERROR] Credit failed: " << db.getLastError() << ". Transaction rolled back.\n";
        db.executeQuery("ROLLBACK");
        return;
    }
    
    // Commit transaction
    if (!db.executeQuery("COMMIT")) {
        cout << "[ERROR] Commit failed: " << db.getLastError() << ". Transaction rolled back.\n";
        db.executeQuery("ROLLBACK");
    } else {
        cout << "[SUCCESS] Transfer of $" << fixed << setprecision(2) << amount 
             << " to " << receiverName << " (ID: " << receiverId << ") completed successfully!\n";
        
        double newBalance = getBalance(session.accountId);
        if (newBalance >= 0) {
            cout << "[BALANCE] Your New Balance: $" << fixed << setprecision(2) << newBalance << "\n";
        }
    }
}

// 6. Check Balance
void BankingService::checkBalance(const models::Session& session) {
    if (!session.isLoggedIn) {
        cout << "[ERROR] You must be logged in to perform this operation.\n";
        return;
    }
    
    double balance = getBalance(session.accountId);
    if (balance < 0) {
        cout << "[ERROR] Error retrieving balance.\n";
    } else {
        cout << "\n[BALANCE] Account Balance for " << session.accountName << " (ID: " << session.accountId << "): $" 
             << fixed << setprecision(2) << balance << "\n";
    }
}

// 7. Update Account Details
void BankingService::updateAccount(models::Session& session) {
    if (!session.isLoggedIn) {
        cout << "[ERROR] You must be logged in to perform this operation.\n";
        return;
    }
    
    cout << "\n=== UPDATE ACCOUNT DETAILS ===\n";
    cout << "1. Update Name\n";
    cout << "2. Update Password\n";
    cout << "3. Cancel\n";
    cout << "Enter your choice: ";
    int choice;
    if (!(cin >> choice)) {
        cout << "[ERROR] Invalid choice.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }
    
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear buffer
    
    if (choice == 1) {
        string newName;
        cout << "Enter New Name: ";
        getline(cin, newName);
        if (newName.empty()) {
            cout << "[ERROR] Name cannot be empty.\n";
            return;
        }
        string safeName = utils::InputSanitizer::sanitize(db, newName);
        string query = "UPDATE accounts SET name = '" + safeName + "' WHERE account_id = " + to_string(session.accountId);
        if (!db.executeQuery(query)) {
            cout << "[ERROR] Error updating name: " << db.getLastError() << "\n";
        } else {
            session.accountName = newName;
            cout << "[SUCCESS] Name updated successfully!\n";
        }
    } else if (choice == 2) {
        string newPassword;
        cout << "Enter New Password: ";
        getline(cin, newPassword);
        if (newPassword.empty()) {
            cout << "[ERROR] Password cannot be empty.\n";
            return;
        }
        string safePassword = utils::InputSanitizer::sanitize(db, newPassword);
        string query = "UPDATE accounts SET password = '" + safePassword + "' WHERE account_id = " + to_string(session.accountId);
        if (!db.executeQuery(query)) {
            cout << "[ERROR] Error updating password: " << db.getLastError() << "\n";
        } else {
            cout << "[SUCCESS] Password updated successfully!\n";
        }
    } else {
        cout << "Update cancelled.\n";
    }
}

// 8. Delete Account
void BankingService::deleteAccount(models::Session& session) {
    if (!session.isLoggedIn) {
        cout << "[ERROR] You must be logged in to perform this operation.\n";
        return;
    }
    
    cout << "\n[WARNING] DANGER: DELETE ACCOUNT\n";
    cout << "Are you absolutely sure you want to delete your account? This action cannot be undone. (y/n): ";
    char confirm;
    cin >> confirm;
    if (confirm != 'y' && confirm != 'Y') {
        cout << "[ERROR] Account deletion cancelled.\n";
        return;
    }
    
    string query = "DELETE FROM accounts WHERE account_id = " + to_string(session.accountId);
    if (!db.executeQuery(query)) {
        cout << "[ERROR] Error deleting account: " << db.getLastError() << "\n";
    } else {
        cout << "[SUCCESS] Account deleted successfully.\n";
        session.logout();
    }
}

// 9. Logout
void BankingService::logout(models::Session& session) {
    if (!session.isLoggedIn) {
        cout << "[ERROR] You are not logged in.\n";
        return;
    }
    cout << "\n[INFO] Logging out. Goodbye, " << session.accountName << "!\n";
    session.logout();
}

} 
