#pragma once
#include <tuple>
#include <proton/registry.hpp>
#include "proton/stage.hpp"
#include "proton/world.hpp"

class myapp {
public:
    using config_type = std::tuple<>;
    static myapp create() { return {}; }
    template <auto World> // single world
    void run() {
        using namespace proton;
        using enum stage;
        auto worlds = make_worlds<World>();
        call_startup(worlds);
        call_update(worlds);
        call<shutdown>(worlds);
    }
};
