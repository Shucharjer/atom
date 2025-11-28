#pragma once
#include "proton/proton.hpp"

#include <neutron/shift_map.hpp>

namespace proton {

template <_std_simple_allocator Alloc>
class world_base {
    template <typename, _std_simple_allocator>
    friend class basic_world;

    friend struct world_accessor;

    template <typename Ty>
    using _allocator_t = neutron::rebind_alloc_t<Alloc, Ty>;

    using archetype = archetype<Alloc>;

public:
    template <typename Al = Alloc>
    constexpr explicit world_base(const Al& alloc = Alloc{})
        : archetypes_(alloc), entities_(alloc) {}

    constexpr entity_t spawn();

    template <component... Components>
    constexpr entity_t spawn();

    template <component... Components>
    constexpr entity_t spawn(Components&&... components);

    template <component... Components>
    constexpr void add_components(entity_t entity);

    template <component... Components>
    constexpr void remove_components(entity_t entity);

    constexpr void kill(entity_t entity);

private:
    ///
    /// vector<archetype>
    std::vector<archetype, _allocator_t<archetype>> archetypes_;
    /// mapping entity to the archetype stores it
    /// shift_map<entity_t, id_t>
    neutron::shift_map<entity_t, index_t, 32UL, sizeof(entity_t) * 4UL, Alloc>
        entities_;
};

template <_std_simple_allocator Alloc>
constexpr entity_t world_base<Alloc>::spawn() {
    return 0;
}

template <_std_simple_allocator Alloc>
template <component... Components>
constexpr entity_t world_base<Alloc>::spawn() {
    return 0;
}

template <_std_simple_allocator Alloc>
template <component... Components>
constexpr entity_t world_base<Alloc>::spawn(Components&&... components) {
    return 0;
}

template <_std_simple_allocator Alloc>
template <component... Components>
constexpr void world_base<Alloc>::add_components(entity_t entity) {
    //
}

template <_std_simple_allocator Alloc>
template <component... Components>
constexpr void world_base<Alloc>::remove_components(entity_t entity) {
    //
}

template <_std_simple_allocator Alloc>
constexpr void world_base<Alloc>::kill(entity_t entity) {
    //
}

} // namespace proton
