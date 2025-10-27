#pragma once
#include "proton/world.hpp"

namespace proton {

template <typename... Args>
class on {
    template <_std_simple_allocator Alloc>
    explicit on(basic_world<Alloc>& alloc) {}
};

} // namespace proton
