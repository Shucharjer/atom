#pragma once
#include "proton/proton.hpp"

namespace proton {

template <auto Sys, typename Ty>
struct call_from_world {
    template <_world World>
    Ty operator()(World& world) {
        return Ty{ world };
    }
};

} // namespace proton
