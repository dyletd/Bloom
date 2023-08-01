#include <iostream>
#include <vector>
#include <string>
#include <bitset>
#include <functional>
#include <fstream>
#include <Windows.h>
#include <sstream>
#include <cmath>
using namespace std;

class BloomFilter {
private:
    bitset<10000> filter; // Fixed size of the Bloom filter
    int numElements; // Number of elements in the filter
    int numHashFunctions; // Number of hash functions

public:
    BloomFilter(int num_elements, double false_positive_rate) : numElements(num_elements) {
        // Calculate the optimal size of the Bloom filter
        int m = -num_elements * log(false_positive_rate) / (log(2) * log(2));
        numHashFunctions = (m / num_elements) * log(2);

        // Ensure that numHashFunctions is at least 1
        numHashFunctions = max(1, numHashFunctions);
    }

    size_t hash1(const string& key) const {
        size_t hashVal = 0;
        for (char c : key) {
            hashVal = 37 * hashVal + c;
        }
        return hashVal % filter.size();
    }

    size_t hash2(const string& key) const {
        size_t hashVal = 5381;
        for (char c : key) {
            hashVal = ((hashVal << 5) + hashVal) + c;
        }
        return hashVal % filter.size();
    }

    void add(const string& key) {
        size_t index1 = hash1(key) % filter.size();
        size_t index2 = hash2(key) % filter.size();
        filter.set(index1);
        filter.set(index2);
    }

    bool contains(const string& key) const {
        size_t index1 = hash1(key) % filter.size();
        size_t index2 = hash2(key) % filter.size();
        return filter.test(index1) && filter.test(index2);
    }

    int getNumHashFunctions() const {
        return numHashFunctions;
    }

    int getBloomFilterSize() const {
        return filter.size();
    }

    void clearFilter() {
        filter.reset(); // Set all bits to 0
    }

};

class AccountManager {
public:
    BloomFilter bloomFilter;

    struct Account {
        string username;
        string password;
    };

    AccountManager() : bloomFilter(1000, 0.01) {} // Số phần tử và tỷ lệ sai tích cực của BloomFilter

    std::vector<Account> accounts;

    void Successed(string username, string password) {
        fstream ofs("Successed.txt", ios::out | ios::app);
        ofs << username << " " << password << endl;
        ofs.close();
    }

    void Failed(string username, string password) {
        fstream ofs("Failed.txt", ios::out | ios::app);
        ofs << username << " " << password << endl;
        ofs.close();
    }

    void ChangeFile(string username, string new_Password) {
        ifstream ifs("Successed.txt");
        if (!ifs) {
            cout << "Failed to open the file.\n";
            return;
        }

        vector<string> lines;
        string line;
        while (getline(ifs, line)) {
            stringstream ss(line);
            string user, pass;
            ss >> user >> pass;

            if (username == user) {
                pass = new_Password;
            }

            lines.push_back(user + " " + pass);
        }
        ifs.close();

        ofstream ofs("Successed.txt");
        if (!ofs) {
            cout << "Failed to open the file.\n";
            return;
        }

        for (const string& l : lines) {
            ofs << l << endl;
        }
        ofs.close();

        cout << "Password changed successfully!\n";
    }

    bool check_Weak(string password) {
        ifstream ifs("Weak.txt");
        string line;
        while (getline(ifs, line)) {
            if (password == line) {
                return false;
            }
        }
        ifs.close();
        return true;
    }

    void registerAccount(const string& username, const string& password) {
        if (bloomFilter.contains(username)) {
            Failed(username, password);
            system("cls");
            cout << "Username already exists. Please choose a different username.\n";
            CreateAcc();
            return;
        }

        int usernameCondition = condit_username(username, password);
        if (usernameCondition != 0) {
            switch (usernameCondition) {
            case 1:
                system("cls");
                cout << "Username must be between 5 and 10 characters" << endl;
                break;
            case 2:
                system("cls");
                cout << "Username must not contain spaces!" << endl;
                break;
            }
            Failed(username, password);
            CreateAcc();
            return;
        }

        int passwordCondition = condit_password(username, password);
        if (passwordCondition != 0) {
            switch (passwordCondition) {
            case 1:
                system("cls");
                cout << "Password must be between 10 and 20 characters!" << endl << endl;
                break;
            case 2:
                system("cls");
                cout << "Password must not contain spaces!" << endl << endl;
                break;
            case 3:
                system("cls");
                cout << "Password must contain at least 1 digit!" << endl << endl;
                break;
            case 4:
                system("cls");
                cout << "Password must contain at least 1 uppercase character!" << endl << endl;
                break;
            case 5:
                system("cls");
                cout << "Password must contain at least 1 special character!" << endl << endl;
                break;
            case 6:
                system("cls");
                cout << "Password must not be the same as the username!" << endl << endl;
                break;
            }
            Failed(username, password);
            CreateAcc();
            return;
        }
        if (check_Weak(password) == false) {
            Failed(username, password);
            system("cls");
            cout << "Password is match the weak password listed in the Weak file!" << endl;
            CreateAcc();
        }

        // Add account to the Bloom Filter
        bloomFilter.add(username);

        // Create and store the account
        Account newAccount;
        newAccount.username = username;
        newAccount.password = password;
        accounts.push_back(newAccount);
        Successed(username, password);
        cout << "Account registered successfully.\n";
        cout << "====================================" << endl << endl;

        char ch;
        cout << " Type \"y\" to Back home or any key to Quit: " << endl;
        cin >> ch;
        cin.ignore();
        if (ch == 'y') {
            system("cls");
            home();
            return;
        }
        else
            return;
    }
    void clear_bloom() {
        bloomFilter.clearFilter();
        accounts.clear();

        ofstream ofs1("Successed.txt", ios::trunc);
        ofstream ofs2("Failed.txt", ios::trunc);
        ofs1.close();
        ofs2.close();
        system("cls");
        cout << "All account information in files and Bloom filter have been deleted.\n" << endl;
        home();
    }


    void push_bloom() {
        ifstream ifs("Successed.txt");
        if (!ifs.is_open())
            return;
        string line, pass, user;
        while (getline(ifs, line)) {
            stringstream ss(line);
            ss >> user >> pass;
            bloomFilter.add(user);

            // Create and store the account
            Account newAccount;
            newAccount.username = user;
            newAccount.password = pass;
            accounts.push_back(newAccount);
        }
        ifs.close();
    }

    bool check_for_multiple(const string& username, const string& password) {
        int usernameCondition = condit_username(username, password);
        int passwordCondition = condit_password(username, password);
        if (bloomFilter.contains(username) || usernameCondition != 0 || passwordCondition != 0 || check_Weak(password) == false)
            return false;
        return true;
    }

    void multiple_register() {
        ifstream ifs("SignUp.txt");
        fstream ofs1("Successed.txt", ios::out | ios::app);
        fstream ofs2("Failed.txt", ios::out | ios::app);
        if (!ifs.is_open()) {
            cout << "Failed to open the file" << endl;
            cout << "Back home after 3s!" << endl;
            Sleep(2000);
            home();
            return;

        }
        string user, pass, line, dummy;
        while (getline(ifs, line)) {
            user = "", pass = "", dummy = "";
            stringstream ss(line);
            ss >> user >> pass >> dummy;//dummy de check xem co khoang trang nao trong ten hoac mk khong
            if (dummy.empty() && check_for_multiple(user, pass) == true) {
                // Add account to the Bloom Filter
                bloomFilter.add(user);

                // Create and store the account
                Account newAccount;
                newAccount.username = user;
                newAccount.password = pass;
                accounts.push_back(newAccount);
                Successed(user, pass);
            }
            else {
                if (dummy.empty())
                    ofs2 << user << " " << pass << endl;
                else
                    ofs2 << user << " " << pass << " " << dummy << endl;
            }


        }
        cout << "Multiple registrtion completed  " << endl;
        cout << "Back home after 3s!" << endl;
        Sleep(2000);
        system("cls");
        home();
        return;
    }
    // Phiên bản không const của hàm findAccountByUsername
    Account* findAccountByUsername(const string& username) {
        for (Account& account : accounts) {
            if (account.username == username) {
                return &account;
            }
        }
        return nullptr;
    }

    // Phiên bản const của hàm findAccountByUsername
    const Account* findAccountByUsername(const string& username) const {
        for (const Account& account : accounts) {
            if (account.username == username) {
                return &account;
            }
        }
        return nullptr;
    }

    void changePass(const string& username, const string& password) {
        string newPassword;
        cout << "===================" << endl;
        cout << "| Change password |" << endl;
        cout << "===================" << endl << endl;

        cout << "Type your new password: ";
        getline(cin, newPassword);
        Account* account = findAccountByUsername(username);
        if (account) {
            int passwordCondition = condit_password(username, newPassword);
            if (passwordCondition != 0) {
                switch (passwordCondition) {
                case 1:
                    system("cls");
                    cout << "Password must be between 10 and 20 characters!" << endl << endl;
                    break;
                case 2:
                    system("cls");
                    cout << "Password must not contain spaces!" << endl << endl;
                    break;
                case 3:
                    system("cls");
                    cout << "Password must contain at least 1 digit!" << endl << endl;
                    break;
                case 4:
                    system("cls");
                    cout << "Password must contain at least 1 uppercase character!" << endl << endl;
                    break;
                case 5:
                    system("cls");
                    cout << "Password must contain at least 1 special character!" << endl << endl;
                    break;
                case 6:
                    system("cls");
                    cout << "Password must not be the same as the username!" << endl << endl;
                    break;
                }
                changePass(username, newPassword);
                return;
            }

            // Thay đổi mật khẩu của tài khoản
            account->password = newPassword;
            cout << "Password has been changed successfully.\n";
            ChangeFile(username, newPassword);
            cout << "Back door after 3s!";
            Sleep(3000);
            system("cls");
            home();
        }

    }


    void login_succ(const string& username, const string& password) {
        char ch;
        cout << "Login successful.\n" << endl;
        cout << "====================" << endl;
        cout << "1.Change password" << endl;
        cout << "====================" << endl;
        cout << "2.Back home" << endl;
        cout << "====================" << endl << endl;
        cout << "Type your choose: ";
        cin >> ch;
        cin.ignore();
        if (ch == '1') {
            system("cls");
            changePass(username, password);
            return;
        }
        else if (ch == '2') {
            system("cls");
            home();
            return;
        }
        else return;
    }

    bool login(const string& username, const string& password) {
        // Check if the username exists in the Bloom Filter
        if (bloomFilter.contains(username)) {
            // Search for the account in the stored accounts
            for (const Account& account : accounts) {
                if (account.username == username && account.password == password) {
                    system("cls");
                    login_succ(username, password);
                    return true;
                }
            }
        }
        char ch;
        cout << endl << "Login failed. Invalid username or password.\n";
        cout << " Type \"y\" to Back home or any key to Quit: " << endl;
        cin >> ch;
        cin.ignore();
        if (ch == 'y') {
            system("cls");
            home();
            return false;
        }
    }

    int condit_password(const string& username, const string& password) const {
        bool upper = false, space = true, digit = false, special = false, same = false;
        vector<char> v;
        for (int i = 0; i < password.size(); i++)
            v.push_back(password[i]);

        if (v.size() < 10 || v.size() > 20) {
            return 1;
        }

        for (int i = 0; i < v.size(); i++) {
            if (v[i] == 32)
                space = false;
            if (v[i] >= 65 && v[i] <= 90)
                upper = true;
            if (v[i] >= 48 && v[i] <= 57)
                digit = true;
            if ((v[i] >= 33 && v[i] <= 47) || (v[i] >= 58 && v[i] <= 64))
                special = true;
        }
        if (space == false)
            return 2;
        if (digit == false)
            return 3;
        if (upper == false)
            return 4;
        if (special == false)
            return 5;
        if (password == username)
            return 6;
        return 0;
    }


    int condit_username(const string& username, const string& password) const {
        vector<char> v;
        for (int i = 0; i < username.size(); i++)
            v.push_back(username[i]);

        if (v.size() < 5 || v.size() > 10) {
            return 1;
        }

        bool space = true;
        for (int i = 0; i < v.size(); i++) {
            if (v[i] == 32)
                space = false;
        }
        if (space == false)
            return 2;
        return 0;
    }

    void CreateAcc() {
        string username;
        string password;
        cout << "===================" << endl;
        cout << "| Register Account |" << endl;
        cout << "===================" << endl << endl;
        cout << "Username (5 to 10 characters): ";
        getline(cin, username);
        cout << "Password (10 to 20 characters): ";
        getline(cin, password);
        registerAccount(username, password);
    }

    void LoginAcc() {
        cout << "===================" << endl;
        cout << "| Login Account    |" << endl;
        cout << "===================" << endl << endl;
        string username, password;
        cout << "Enter your username: ";
        getline(cin, username);
        cout << "Enter your password: ";
        getline(cin, password);
        login(username, password);
    }

    void home() {
        push_bloom();
        cout << "========================" << endl;
        cout << "1.Register new account" << endl;
        cout << "========================" << endl;
        cout << "2.Login" << endl;
        cout << "========================" << endl;
        cout << "3.Multiple register     " << endl;
        cout << "========================" << endl;
        cout << "4.Delete all account " << endl;
        cout << "========================" << endl;
        cout << "5.Close" << endl;
        cout << "========================" << endl;


        int choose;
        cout << "Enter your choose (1, 2, v.v):  ";
        cin >> choose;
        cin.ignore();
        switch (choose) {
        case 1:
            system("cls");
            CreateAcc();
            break;
        case 2:
            system("cls");
            LoginAcc();
            break;
        case 3:
            system("cls");
            multiple_register();
            break;
        case 4:
            char ch;
            cout << "Do you want to delete all account?(\"y\" for YES or any key for NO): ";
            cin >> ch;
            if (ch == 'y')
                clear_bloom();
            else {
                system("cls");
                home();
                return;
            }

            break;
        case 5:
            return;
        default:
            system("cls");
            cout << "Invalid choice. Please try again.\n";
            home();
            break;
        }
    }

};

int main() {
    int num_elements = 1000; // Expected number of elements in the filter
    double false_positive_rate = 0.01; // Desired false positive rate

    AccountManager manager;
    manager.home();
    return 0;
}
