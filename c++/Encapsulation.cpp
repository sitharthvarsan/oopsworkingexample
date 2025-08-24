#include <iostream>
using namespace std;

class BankAccount {
private:
    int balance; // Data is hidden

public:
    void deposit(int amount) {
        balance += amount;
    }
    int getBalance() {
        return balance;
    }
};

int main() {
    BankAccount acc;
    acc.deposit(500);              
    cout << acc.getBalance();      
        return 0;
}

