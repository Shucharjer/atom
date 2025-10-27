#pragma once
#include <iostream>

class myapp {
public:
    static myapp create() { return {}; }
    template <auto World> // single world
    void run() {
        std::cout << "hello world\n";
    }
};
