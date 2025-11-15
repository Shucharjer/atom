#pragma once
#include "proton/world.hpp"

namespace proton {

template <typename Event>
class on {
public:
    template <_world World>
    explicit on(World& world) {}

    decltype(auto) event() noexcept { return; }
    decltype(auto) event() const noexcept { return; }
};

} // namespace proton
