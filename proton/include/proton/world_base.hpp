#pragma once
#include "proton/proton.hpp"

#include <version>
#ifndef HAS_STD_FLAT_MAP
    #if defined(__cpp_lib_flat_map) && __cpp_lib_flat_map >= 202207L
        #define HAS_STD_FLAT_MAP 1
    #else
        #define HAS_STD_FLAT_MAP 0
    #endif
#endif

#if HAS_STD_FLAT_MAP
    #include <flat_map>
#endif

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <queue>
#include <type_traits>
#include <utility>
#include <vector>
#include <neutron/memory.hpp>
#include <neutron/shift_map.hpp>
#include <neutron/template_list.hpp>
#include <neutron/type_hash.hpp>
#include "proton/archetype.hpp"

namespace proton {

struct world_accessor;

namespace _world_base {

#ifndef PROTON_DISABLE_GENERATION

template <_std_simple_allocator Alloc = std::allocator<std::byte>>
class world_base {
    template <typename, _std_simple_allocator>
    friend class basic_world;

    friend struct ::proton::world_accessor;

    template <typename Ty>
    using _allocator_t = neutron::rebind_alloc_t<Alloc, Ty>;

    template <typename Ty>
    using _vector_t = std::vector<Ty, _allocator_t<Ty>>;

    using archetype = archetype<Alloc>;

    #if HAS_STD_FLAT_MAP && false // flat_map is a not stable storage
    using archetype_map = std::flat_map<
        uint64_t, archetype, std::less<uint64_t>, _vector_t<uint64_t>,
        _vector_t<archetype>>;
    #else
    using archetype_map = std::unordered_map<
        uint64_t, archetype, std::hash<uint64_t>, std::equal_to<uint64_t>,
        _allocator_t<std::pair<const uint64_t, archetype>>>;
    #endif

    template <typename Ty>
    using _priority_queue = std::priority_queue<Ty, _vector_t<Ty>>;

public:
    using size_type = size_t;

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

    constexpr void reserve(size_type n);

    template <component... Components>
    constexpr void reserve(size_type n);

    constexpr void clear();

private:
    constexpr entity_t _get_new_entity();
    template <component... Components>
    constexpr void _emplace_new_entity(entity_t entity);
    template <component... Components>
    constexpr void _emplace_new_entity(entity_t entity, Components&&...);

    archetype_map archetypes_;
    /// mapping entity to the archetype stores it
    neutron::shift_map<
        entity_t, archetype*, 1024UL, neutron::half_bits<entity_t>, Alloc>
        entities_;
    _vector_t<generation_t> generations_;
    _priority_queue<uint32_t> free_indices_;
};

ATOM_FORCE_INLINE static constexpr generation_t
    _get_gen(entity_t entity) noexcept {
    return static_cast<generation_t>(entity >> 32U);
}

ATOM_FORCE_INLINE constexpr static index_t
    _get_index(entity_t entity) noexcept {
    return static_cast<index_t>(entity);
}

NODISCARD ATOM_FORCE_INLINE constexpr static entity_t
    _make_entity(generation_t gen, index_t index) noexcept {
    return (static_cast<entity_t>(gen) << 32UL) | index;
}

template <_std_simple_allocator Alloc>
constexpr entity_t world_base<Alloc>::_get_new_entity() {
    if (free_indices_.empty()) {
        generations_.emplace_back();
        return generations_.size() - 1;
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
template <component... Components>
constexpr void world_base<Alloc>::_emplace_new_entity(entity_t entity) {
    using namespace neutron;
    using list              = type_list<std::remove_cvref_t<Components>...>;
    constexpr uint64_t hash = make_array_hash<list>();

    auto iter = archetypes_.find(hash);
    if (iter != archetypes_.end()) {
        iter->second.emplace(entity);
        entities_.try_emplace(entity, &iter->second);
    } else {
        auto [iter, _] = archetypes_.try_emplace(
            hash, archetype{ spread_type<Components...> });
        iter->second.emplace(entity);
        entities_.try_emplace(entity, &iter->second);
    }
}

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
constexpr void world_base<Alloc>::_emplace_new_entity(
    entity_t entity, Components&&... components) {
    using namespace neutron;
    using list              = type_list<std::remove_cvref_t<Components>...>;
    constexpr uint64_t hash = make_array_hash<list>();

    auto iter = archetypes_.find(hash);
    if (iter != archetypes_.end()) [[likely]] {
        iter->second.emplace(entity, std::forward<Components>(components)...);
        entities_.try_emplace(entity, &iter->second);
    } else [[unlikely]] {
        auto [iter, _] = archetypes_.try_emplace(
            hash, archetype{ spread_type<std::remove_cvref_t<Components>...> });
        iter->second.emplace(entity, std::forward<Components>(components)...);
        entities_.try_emplace(entity, &iter->second);
    }
}

template <_std_simple_allocator Alloc>
template <component... Components>
constexpr entity_t world_base<Alloc>::spawn(Components&&... components) {
    constexpr uint64_t hash = neutron::make_array_hash<
        neutron::type_list<std::remove_cvref_t<Components>...>>();
    const auto entity = _get_new_entity();
    _emplace_new_entity(entity, std::forward<Components>(components)...);
    return entity;
}

template <_std_simple_allocator Alloc>
template <component... Components>
constexpr void world_base<Alloc>::add_components(entity_t entity) {
    constexpr uint64_t hash = neutron::make_array_hash<
        neutron::type_list<std::remove_cvref_t<Components>...>>();
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
    auto* const arche = entities_.at(entity);
    if (arche != nullptr) {
        arche->erase(entity);
    }
    entities_.erase(entity);
    free_indices_.push(_get_index(entity));
}

template <_std_simple_allocator Alloc>
constexpr void world_base<Alloc>::reserve(size_type n) {
    entities_.reserve(n);
    generations_.reserve(n);
}

template <_std_simple_allocator Alloc>
template <component... Components>
constexpr void world_base<Alloc>::reserve(size_type n) {
    using namespace neutron;
    using list          = type_list<Components...>;
    constexpr auto hash = make_array_hash<list>();

    if (auto iter = archetypes_.find(hash); iter != archetypes_.end()) {
        iter->second.reserve(n);
    }

    entities_.reserve(n);
    generations_.reserve(n);
}

template <_std_simple_allocator Alloc>
constexpr void world_base<Alloc>::clear() {
    for (auto& [hash, arche] : archetypes_) {
        for (auto entity : arche.entities()) {
            kill(entity);
        }
    }
}

#else

// TODO: extremely fast world_base

template <_std_simple_allocator Alloc = std::allocator<std::byte>>
class world_base {
public:
private:
};

#endif

} // namespace _world_base

template <_std_simple_allocator Alloc = std::allocator<std::byte>>
using world_base = _world_base::world_base<Alloc>;

} // namespace proton
