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

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
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

/**
 * @class world_base
 * @brief A container stores entities and their component data.
 *
 * The index of an entity starts from 1.
 * @tparam Alloc Allocator satisfy the contrains of allocator by the standard
 * library. Default is `std::allocator<std::byte>`. The container would rebind
 * allocator automatically.
 */
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
    explicit world_base(const Al& alloc = Alloc{})
        : archetypes_(alloc), entities_(1, alloc) {}

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

    constexpr bool is_alive(entity_t entity) noexcept;

    void clear();

private:
    constexpr entity_t _get_new_entity();
    template <component... Components>
    constexpr void _emplace_new_entity(entity_t entity);
    template <component... Components>
    constexpr void _emplace_new_entity(entity_t entity, Components&&...);

    /// @brief A container stores archetypes with combined hash.
    archetype_map archetypes_;
    /// @brief A stroage mapping index to entity and its archetype.
    /// We do never pop or erase: when killing a entity, we set index zero
    /// and its archetype pointer null.
    /// Summary: for an entity ((gen, index), p_archetype),
    /// ((gen, non-zero), nullptr) -> created, but has no component
    /// ((gen, 0), 0) -> killed entity
    _vector_t<std::pair<entity_t, archetype*>> entities_;
    /// @brief A priority queue stores free indices.
    /// It makes us have the ability to get the smallest index all the time.
    /// A smaller index means we could access `sparse_map` or `shift_map` with
    /// lower frequency of cache missing.
    _priority_queue<uint32_t> free_indices_;
};

ATOM_FORCE_INLINE static constexpr generation_t
    _get_gen(entity_t entity) noexcept {
    return static_cast<generation_t>(entity >> 32U);
}

ATOM_FORCE_INLINE static constexpr index_t
    _get_index(entity_t entity) noexcept {
    return static_cast<index_t>(entity);
}

ATOM_FORCE_INLINE static constexpr void
    _reset_index(entity_t& entity) noexcept {
    entity = ((entity >> 32) << 32);
}

NODISCARD ATOM_FORCE_INLINE constexpr static entity_t
    _make_entity(generation_t gen, index_t index) noexcept {
    return (static_cast<entity_t>(gen) << 32UL) | index;
}

template <_std_simple_allocator Alloc>
constexpr entity_t world_base<Alloc>::_get_new_entity() {
    if (free_indices_.empty()) {
        const auto index = entities_.size();
        entities_.emplace_back(index, nullptr); // _make_entity(0, index)
        return index;
    }
    const index_t index = free_indices_.top();
    free_indices_.pop();
    const generation_t gen = _get_gen(entities_[index].first) + 1;
    entities_[index].first = _make_entity(gen, index);
    return entities_[index].first;
}

template <_std_simple_allocator Alloc>
constexpr entity_t world_base<Alloc>::spawn() {
    return _get_new_entity();
}

template <_std_simple_allocator Alloc>
template <component... Components>
constexpr void world_base<Alloc>::_emplace_new_entity(entity_t entity) {
    using namespace neutron;
    using list              = type_list<Components...>;
    constexpr uint64_t hash = make_array_hash<list>();

    const auto index = _get_index(entity);
    auto iter        = archetypes_.find(hash);
    if (iter != archetypes_.end()) {
        iter->second.template emplace<Components...>(entity);
        entities_[index].second = &iter->second;
    } else {
        auto [iter, _] = archetypes_.try_emplace(
            hash, archetype{ spread_type<Components...> });
        iter->second.template emplace<Components...>(entity);
        entities_[index].second = &iter->second;
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

    const auto index = _get_index(entity);
    auto iter        = archetypes_.find(hash);
    if (iter != archetypes_.end()) [[likely]] {
        iter->second.emplace(entity, std::forward<Components>(components)...);
        entities_[index].second = &iter->second;
    } else [[unlikely]] {
        auto [iter, _] = archetypes_.try_emplace(
            hash, archetype{ spread_type<std::remove_cvref_t<Components>...> });
        iter->second.emplace(entity, std::forward<Components>(components)...);
        entities_[index].second = &iter->second;
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
    const auto index = _get_index(entity);
    if (entities_[index].second == nullptr) {
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
    const auto index = _get_index(entity);
    if (entities_[index].second == nullptr) {
        _emplace_new_entity<Components...>(
            entity, std::forward<Components>(components)...);
        return;
    }

    // move a entity from an archetype to another
}

template <_std_simple_allocator Alloc>
template <component... Components>
constexpr void world_base<Alloc>::remove_components(entity_t entity) {
    const auto index = _get_index(entity);
    auto*& arche     = entities_[index].second;
    if (arche == nullptr) [[unlikely]] {
        return;
    }

    // remove components
}

template <_std_simple_allocator Alloc>
constexpr void world_base<Alloc>::kill(entity_t entity) {
    const auto index = _get_index(entity);
    const auto gen   = _get_gen(entity);
    assert(entity == entities_[index].first);

    auto*& arche = entities_[index].second;
    if (arche != nullptr) {
        arche->erase(entity);
        arche = nullptr;
    }
    _reset_index(entities_[index].first);
    if (gen != (std::numeric_limits<uint32_t>::max)()) [[likely]] {
        free_indices_.push(index);
    }
}

template <_std_simple_allocator Alloc>
constexpr void world_base<Alloc>::reserve(size_type n) {
    entities_.reserve(n);
}

template <_std_simple_allocator Alloc>
template <component... Components>
constexpr void world_base<Alloc>::reserve(size_type n) {
    using namespace neutron;
    using list          = type_list<Components...>;
    constexpr auto hash = make_array_hash<list>();

    if (auto iter = archetypes_.find(hash); iter != archetypes_.end()) {
        iter->second.reserve(n);
    } else {
        auto [it, succ] =
            archetypes_.emplace(hash, archetype{ spread_type<Components...> });
        it->second.reserve(n);
    }

    entities_.reserve(n);
}

template <_std_simple_allocator Alloc>
constexpr bool world_base<Alloc>::is_alive(entity_t entity) noexcept {
    const auto index = _get_index(entity);
    return index != 0 && entities_.size() > index;
}

template <_std_simple_allocator Alloc>
void world_base<Alloc>::clear() {
    for (auto& [_, archetype] : archetypes_) {
        archetype.clear();
    }
    free_indices_ = _priority_queue<index_t>{ entities_.get_allocator() };
    entities_.clear();
    entities_.emplace_back();
}

} // namespace _world_base

template <_std_simple_allocator Alloc = std::allocator<std::byte>>
using world_base = _world_base::world_base<Alloc>;

} // namespace proton
