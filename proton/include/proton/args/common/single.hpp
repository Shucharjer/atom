#pragma once
#include "proton/proton.hpp"

namespace proton {

template <typename... Args>
class single {
public:
    template <_world World>
    single(World& world) {}
};

} // namespace proton
