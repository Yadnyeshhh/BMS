#include "src/database/DatabaseManager.hpp"
#include "src/models/Session.hpp"
#include "src/services/BankingService.hpp"
#include <iostream>
#include <limits>
#include <fstream>
#include <sstream>

using namespace std;

// env 
void loadEnv(const string& filename, string& host, string& user, string& password, int& port) {
    ifstream file(filename);
    if (!file.is_open()) return;
    
    string line;
    while (getline(file, line)) {
        // Trim leading spaces
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == string::npos || line[first] == '#') continue;
        
        size_t pos = line.find('=');
        if (pos == string::npos) continue;
        
        string key = line.substr(0, pos);
        string val = line.substr(pos + 1);
        
        key.erase(0, key.find_first_not_of(" \t\r\n\""));
        size_t key_last = key.find_last_not_of(" \t\r\n\"");
        if (key_last != string::npos) key.erase(key_last + 1);
        
        val.erase(0, val.find_first_not_of(" \t\r\n\""));
        size_t val_last = val.find_last_not_of(" \t\r\n\"");
        if (val_last != string::npos) val.erase(val_last + 1);
        
        if (key == "DB_HOST") host = val;
        else if (key == "DB_USER") user = val;
        else if (key == "DB_PASSWORD") password = val;
        else if (key == "DB_PORT") {
            try { port = stoi(val); } catch (...) {}
        }
    }
}

int main() {
    cout << "===========================================\n";
    cout << "       WELCOME TO THE BANKING SYSTEM       \n";
    cout << "===========================================\n";

    string dbHost = "localhost";
    string dbUser = "root";
    string dbPassword = "Y@du2048";
    int dbPort = 3306;

    loadEnv(".env", dbHost, dbUser, dbPassword, dbPort);

    database::DatabaseManager db;
    if (!db.connect(dbHost, dbUser, dbPassword, dbPort)) {
        cout << "xxxxx [ERROR] Database Connection failed:  xxxxx " << db.getLastError() << "\n";
        cout << "xxxxx Please ensure MySQL Server is running with the correct credentials. xxxxx\n";
        cout << "xxxxx Press Enter to exit... xxxxx";
        cin.get();
        return 1;
    }

    if (!db.initializeSchema()) {
        cout << "[ERROR] Database Initialization failed: " << db.getLastError() << "\n";
        cout << "Press Enter to exit...";
        cin.get();
        return 1;
    }

    cout << "[SUCCESS] Connected to MySQL and Database initialized successfully.\n";

    services::BankingService banking(db);
    models::Session session;

    int choice = 0;
    while (true) {
        cout << "\n===========================================\n";
        if (session.isLoggedIn) {
            cout << " Logged in as: " << session.accountName << " (ID: " << session.accountId << ")\n";
            cout << "===========================================\n";
            cout << "1. Deposit Money\n";
            cout << "2. Withdraw Money\n";
            cout << "3. Transfer Money\n";
            cout << "4. Check Balance\n";
            cout << "5. Update Account Details\n";
            cout << "6. Delete Account\n";
            cout << "7. Logout\n";
            cout << "8. Exit\n";
        } else {
            cout << " Not Logged In\n";
            cout << "===========================================\n";
            cout << "1. Create Account\n";
            cout << "2. Login\n";
            cout << "3. Exit\n";
        }
        cout << "===========================================\n";
        cout << "Enter your choice: ";

        if (!(cin >> choice)) {
            cout << " xxxxx [ERROR] Invalid input. Please enter a number. xxxxx\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        if (session.isLoggedIn) {
            switch (choice) {
                case 1: banking.deposit(session); break;
                case 2: banking.withdraw(session); break;
                case 3: banking.transfer(session); break;
                case 4: banking.checkBalance(session); break;
                case 5: banking.updateAccount(session); break;
                case 6: banking.deleteAccount(session); break;
                case 7: banking.logout(session); break;
                case 8:
                    cout << "\nThank you for using the Banking System. Goodbye!\n";
                    return 0;
                default:
                    cout << "[ERROR] Invalid choice. Please select between 1 and 8.\n";
            }
        } else {
            switch (choice) {
                case 1: banking.createAccount(); break;
                case 2: banking.login(session); break;
                case 3:
                    cout << "\nThank you for using the Banking System. Goodbye!\n";
                    return 0;
                default:
                    cout << "[ERROR] Invalid choice. Please select between 1 and 3.\n";
            }
        }
    }

    return 0;
}