#pragma once
#include <cstdint>
#include <utility>
#include "proton/proton.hpp"
#include "proton/world.hpp"

namespace proton {

class commands {
public:
    template <_std_simple_allocator Alloc>
    explicit commands(basic_world<Alloc>& world) {}

    std::pair<uint64_t, bool> spwan();

    template <component... Components>
    std::pair<uint64_t, bool> spwan();

    void kill(uint64_t entity);
};

} // namespace proton
