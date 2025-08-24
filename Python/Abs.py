from abc import ABC, abstractmethod

class PaymentMethod(ABC):
    @abstractmethod
    def pay(self, amount):
        pass

class CardPayment(PaymentMethod):
    def pay(self, amount):
        print(f"Paid {amount} with card")

class UpiPayment(PaymentMethod):
    def pay(self, amount):
        print(f"Paid {amount} with UPI")

payment = CardPayment()
payment.pay(100)  
