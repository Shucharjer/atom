#pragma once
#include "proton/proton.hpp"

#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <new>
#include <type_traits>
#include <vector>
#include "proton/world_base.hpp"

namespace proton {

/**
 * @class command_buffer
 * @brief A buffer stores commands in a single thread.
 *
 * @tparam Alloc
 */
template <_std_simple_allocator Alloc = std::allocator<std::byte>>
class alignas(std::hardware_destructive_interference_size) command_buffer {

    template <typename Ty>
    using _allocator_t = neutron::rebind_alloc_t<Alloc, Ty>;

    template <typename Ty>
    using _vector_t = std::vector<Ty, _allocator_t<Ty>>;

    using _world_base = world_base<Alloc>;

public:
    template <typename Al = Alloc>
    constexpr command_buffer(const Al& alloc = {}) : commands_(alloc) {}

    constexpr void reset() noexcept {
        inframe_index_ = 0;
        commands_.clear();
    }

    constexpr future_entity_t spawn() noexcept {
        return future_entity_t{ inframe_index_++ };
    }

    template <component... Components>
    requires(std::same_as<Components, std::remove_cvref_t<Components>> && ...)
    future_entity_t spawn() {
        const auto fut = future_entity_t{ inframe_index_++ };
        //
        return fut;
    }

    template <component... Components>
    future_entity_t spawn(Components&&... components) {
        const auto fut = future_entity_t{ inframe_index_++ };
        //
        return fut;
    }

    template <component... Components>
    void add_components(future_entity_t entity);

    template <component... Components>
    void add_components(entity_t entity) {
        commands_.emplace_back([entity](_world_base& world) {
            world.template add_components<Components...>(entity);
        });
    }

    template <component... Components>
    void add_components(future_entity_t entity, Components&&... comopnents);

    template <component... Components>
    void add_components(entity_t entity, Components&&... components) {
        commands_.emplace_back([entity, &components...](_world_base& world) {
            world.add_components(
                entity, std::forward<Components>(components)...);
        });
    }

    template <component... Components>
    void remove_components(future_entity_t entity);

    template <component... Components>
    void remove_components(entity_t entity) {
        commands_.emplace_back([entity](_world_base& world) {
            world.template remove_components<Components...>(entity);
        });
    }

    void kill(future_entity_t entity);

    void kill(entity_t entity) {
        commands_.emplace_back(
            [entity](_world_base& world) { world.kill(entity); });
    }

    void apply(world_base<Alloc>& world) {
        _vector_t<entity_t> future_map(
            inframe_index_, commands_.get_allocator());

        for (auto& cmd : commands_) {
            cmd(world);
        }
    }

private:
    index_t inframe_index_{};
    _vector_t<std::function<void(world_base<Alloc>&)>> commands_;
};

} // namespace proton
