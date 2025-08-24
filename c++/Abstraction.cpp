#include <iostream>
using namespace std;

class CoffeeMachine {
public:
    void makeCoffee() {
        boilWater();
        brew();
        serve();
    }

private:
    void boilWater() { /* hidden details */ }
    void brew() { cout << "Brewing coffee..." << endl; }
    void serve() { cout << "Serving coffee." << endl; }
};

int main() {
    CoffeeMachine cm;
    cm.makeCoffee(); 
    return 0;
}

