#pragma once
#include <cstdint>
#include <utility>
#include "proton/proton.hpp"
#include "proton/world.hpp"

namespace proton {

class commands {
public:
    template <_world World>
    explicit commands(World& world) {}

    std::pair<uint64_t, bool> spawn();

    template <component... Components>
    std::pair<uint64_t, bool> spawn();

    void kill(uint64_t entity);
};

} // namespace proton
