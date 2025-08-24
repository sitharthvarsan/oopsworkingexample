#include <iostream>
using namespace std;

class Animal {
public:
    void eat() { cout << "Animal eats." << endl; }
};

class Dog : public Animal { // Dog inherits from Animal
public:
    void bark() { cout << "Dog barks." << endl; }
};

int main() {
    Dog d;
    d.eat();  // Can use Animal's method
    d.bark(); // Its own method
    return 0;
}
// Output:
// Animal eats.
// Dog barks.
