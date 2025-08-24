class BankAccount:
    def __init__(self):
        self.__balance = 0  # private
    
    def deposit(self, amount):
        if amount > 0:
            self.__balance += amount
    
    def get_balance(self):
        return self.__balance

account = BankAccount()
account.deposit(500)
print(account.get_balance())  # Output: 500
