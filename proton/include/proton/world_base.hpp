#pragma once
#include "proton/proton.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <queue>
#include <utility>
#include <vector>
#include <neutron/memory.hpp>
#include <neutron/shift_map.hpp>
#include "neutron/type_hash.hpp"
#include "proton/archetype.hpp"

namespace proton {

struct world_accessor;

namespace _world_base {
template <_std_simple_allocator Alloc = std::allocator<std::byte>>
class world_base {
    template <typename, _std_simple_allocator>
    friend class basic_world;

    friend struct ::proton::world_accessor;

    template <typename Ty>
    using _allocator_t = neutron::rebind_alloc_t<Alloc, Ty>;

    template <typename Ty>
    using _vector_t = std::vector<Ty, _allocator_t<Ty>>;

    template <
        typename Kty, typename Ty, typename Hash = std::hash<Kty>,
        typename Pred = std::equal_to<Kty>>
    using _unordered_map = std::unordered_map<
        Kty, Ty, Hash, Pred, _allocator_t<std::pair<const Kty, Ty>>>;

    template <typename Ty>
    using _priority_queue = std::priority_queue<Ty, _vector_t<Ty>>;

public:
    using archetype = archetype<Alloc>;

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
    constexpr void add_components(entity_t entity, Components&&... components);

    template <component... Components>
    constexpr void remove_components(entity_t entity);

    constexpr void kill(entity_t entity);

private:
    constexpr entity_t _get_new_entity();
    template <typename... Components>
    constexpr void _emplace_new_entity(entity_t entity);
    template <typename... Components>
    constexpr void _emplace_new_entity(entity_t entity, Components&&...);

    NODISCARD constexpr entity_t
        _make_entity(generation_t gen, index_t index) const noexcept {
        return (static_cast<entity_t>(gen) << 32UL) | index;
    }

    _unordered_map<uint64_t, archetype> archetypes_;
    /// mapping entity to the archetype stores it
    neutron::shift_map<
        entity_t, archetype*, 32UL, sizeof(entity_t) * 4UL, Alloc>
        entities_;
    _vector_t<generation_t> generations_;
    _priority_queue<uint32_t> free_indices_;

    // for fast query

    _unordered_map<uint64_t, _vector_t<archetype*>> combined_archetypes_;
};

ATOM_FORCE_INLINE constexpr static index_t
    _get_index(entity_t entity) noexcept {
    return static_cast<index_t>(entity);
}

template <_std_simple_allocator Alloc>
constexpr entity_t world_base<Alloc>::_get_new_entity() {
    if (free_indices_.empty()) {
        return entities_.size();
    }
    const index_t index = free_indices_.top();
    free_indices_.pop();
    return _make_entity(++generations_[index], index);
}

template <_std_simple_allocator Alloc>
constexpr entity_t world_base<Alloc>::spawn() {
    const auto entity = _get_new_entity();
    entities_.try_emplace(entity, nullptr);
    return entity;
}

template <_std_simple_allocator Alloc>
template <typename... Components>
constexpr void world_base<Alloc>::_emplace_new_entity(entity_t entity) {}

template <_std_simple_allocator Alloc>
template <component... Components>
constexpr entity_t world_base<Alloc>::spawn() {
    constexpr uint64_t hash =
        neutron::make_array_hash<neutron::type_list<Components...>>();
    const auto entity = _get_new_entity();
    _emplace_new_entity<Components...>(entity);
    return entity;
}

template <_std_simple_allocator Alloc>
template <component... Components>
constexpr entity_t world_base<Alloc>::spawn(Components&&... components) {
    constexpr uint64_t hash =
        neutron::make_array_hash<neutron::type_list<Components...>>();
    const auto entity = _get_new_entity();
    _emplace_new_entity(entity, std::forward<Components>(components)...);
    return entity;
}

template <_std_simple_allocator Alloc>
template <component... Components>
constexpr void world_base<Alloc>::add_components(entity_t entity) {
    constexpr uint64_t hash =
        neutron::make_array_hash<neutron::type_list<Components...>>();
    if (entities_.at(entity) == nullptr) {
        _emplace_new_entity<Components...>(entity);
        return;
    }

    // move a entity from an archetype to another
}

template <_std_simple_allocator Alloc>
template <component... Components>
constexpr void world_base<Alloc>::add_components(
    entity_t entity, Components&&... components) {
    constexpr uint64_t hash =
        neutron::make_array_hash<neutron::type_list<Components...>>();
    if (entities_.at(entity) == nullptr) {
        _emplace_new_entity<Components...>(
            entity, std::forward<Components>(components)...);
        return;
    }

    // move a entity from an archetype to another
}

template <_std_simple_allocator Alloc>
template <component... Components>
constexpr void world_base<Alloc>::remove_components(entity_t entity) {
    auto arche = entities_.at(entity);
    if (arche == nullptr) [[unlikely]] {
        return;
    }

    // remove components
}

template <_std_simple_allocator Alloc>
constexpr void world_base<Alloc>::kill(entity_t entity) {
    auto* arche = entities_.at(entity);
    if (arche != nullptr) {
        arche->erase(entity);
    }
    entities_.erase(entity);
    free_indices_.push(_get_index(entity));
}

} // namespace _world_base

template <_std_simple_allocator Alloc = std::allocator<std::byte>>
using world_base = _world_base::world_base<Alloc>;

} // namespace proton
