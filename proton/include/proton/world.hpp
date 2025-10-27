#pragma once
#include <vector>
#include <neutron/shift_map.hpp>
#include "args/common/command_buffer.hpp"
#include "proton/archetype.hpp"
#include "proton/proton.hpp"


namespace proton {

template <_std_simple_allocator Alloc = std::allocator<archetype>>
class basic_world {
    friend struct world_accessor;

public:
    basic_world(const Alloc& alloc = Alloc{}) : archetypes_(alloc), entities_(alloc) {}

private:
    std::vector<archetype, Alloc> archetypes_;
    neutron::shift_map<uint64_t, uint32_t, Alloc> entities_;
    basic_command_buffer<Alloc> command_buffers_;
};

struct world_accessor {
    template <_std_simple_allocator Alloc>
    static auto& archetypes(basic_world<Alloc>& world) noexcept {
        return world.archetypes_;
    }
    template <_std_simple_allocator Alloc>
    static auto& entities(basic_world<Alloc>& world) noexcept {
        return world.entities_;
    }
};

using world = basic_world<>;

} // namespace proton
