#pragma once
#include "proton/proton.hpp"

namespace proton {

template <auto Sys, typename Ty>
struct make_from_world {
    template <world World>
    Ty operator()(World& world) {
        return Ty{ world };
    }
};

} // namespace proton
