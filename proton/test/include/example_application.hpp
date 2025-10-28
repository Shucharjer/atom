#pragma once
#include <iostream>
#include <tuple>

class myapp {
public:
    using config_type = std::tuple<>;
    static myapp create() { return {}; }
    template <auto World> // single world
    void run() {
        std::cout << "hello world\n";
    }
};
