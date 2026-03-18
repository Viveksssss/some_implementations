#include "unique_ptr.hpp"
#include <iostream>
#include <string>
#include <vector>

struct animal {
    std::string name = "animal";
    virtual void speak() = 0;
    virtual ~animal() = default;
};

struct cat : animal {
    cat() {}

    void speak() override {
        name = "cat";
        std::cout << name << std::endl;
    }
};

struct dog : animal {
    dog() {}

    void speak() override {
        name = "dog";
        std::cout << name << std::endl;
    }
};

int main(int, char **) {
    std::vector<unique_ptr<animal>> vec;
    vec.push_back(make_unique<cat>());
    vec.push_back(make_unique<dog>());

    for (auto &p: vec) {
        p->speak();
    }
}
