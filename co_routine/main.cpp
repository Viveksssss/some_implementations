#include "learn_coroutine.hpp"
#include <chrono>
#include <iostream>
#include <thread>


Task task1() {
    std::cout << "task1 start\n";
    std::cout << co_await await{5} << std::endl;
    std::cout << "ues\n";
    co_return ; 
}

int main(int, char **) {
    std::cout << "Hello, from co_routine!\n";
    auto t = task1();
    std::cout << "Hello, from co_routine!\n";
    // std::this_thread::sleep_for(std::chrono::seconds(6));
    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(6));
    }
}
