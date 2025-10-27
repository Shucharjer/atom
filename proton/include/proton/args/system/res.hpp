#pragma once
#include "proton/proton.hpp"
#include "proton/world.hpp"

namespace proton {

template <resource... Resources>
class res {
public:
    template <_std_simple_allocator Alloc>
    res(basic_world<Alloc>& world);
};

} // namespace proton
