#include <iostream>

#include "generator.h"


template<typename T>
generator<int> range(T first, T last) {
    while (first != last) {
        co_yield first++;
    }
}


generator<int> fibbonachi() {
    long long a = 0;
    long long b = 1;
    while (true) {
        co_yield b;
        auto tmp = a;
        a = b;
        b += tmp;
    }
}

int main() {
    for (auto i: range(0, 10)) {
        std::cout << i << '\n';
    }
    for (int i: fibbonachi()) {
        if (i > 1'000'000) break;
        std::cout << i << ' ';
    }
    std::cout << '\n';


}