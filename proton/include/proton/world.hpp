#pragma once
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include <neutron/memory.hpp>
#include <neutron/shared_tuple.hpp>
#include <neutron/shift_map.hpp>
#include <neutron/type_hash.hpp>
#include "proton/archetype.hpp"
#include "proton/args/make_from_world.hpp"
#include "proton/command_buffer.hpp"
#include "proton/proton.hpp"
#include "proton/stage.hpp"
#include "proton/system.hpp"

namespace proton {

template <_std_simple_allocator Alloc>
class world_base {
    template <
        typename, typename, typename, typename, typename, _std_simple_allocator>
    friend class basic_world;

    friend struct world_accessor;

    template <typename Ty>
    using _rebind_alloc_t = neutron::rebind_alloc_t<Alloc, Ty>;

    using archetype = archetype<Alloc>;

public:
    template <typename Al = Alloc>
    constexpr world_base(const Al& alloc = Alloc{})
        : archetypes_(alloc), entities_(alloc) {}

    constexpr future_entity_t spawn();

    template <typename... Components>
    constexpr future_entity_t spawn();

    template <typename... Components>
    constexpr future_entity_t spawn(Components&&... components);

    template <typename... Components>
    constexpr void add_components(entity_t entity);

    template <typename... Components>
    constexpr void add_components(future_entity_t entity);

    template <typename... Components>
    constexpr void remove_components(entity_t entity);

    template <typename... Components>
    constexpr void remove_components(future_entity_t entity);

    constexpr void kill(entity_t entity);

    constexpr void kill(future_entity_t entity);

private:
    ///
    /// vector<archetype>
    std::vector<archetype, _rebind_alloc_t<archetype>> archetypes_;
    /// mapping entity to the archetype stores it
    /// shift_map<entity_t, id_t>
    neutron::shift_map<entity_t, index_t, 32UL, sizeof(entity_t) * 4UL, Alloc>
        entities_;
};

// clang-format off
template <
    template <typename...> typename CompList, _comp_or_bundle... Comps,
    typename SystemLists,
    typename Observers,
    typename... Locals,
    template <typename...> typename ResList, _res_or_bundle... Reses,
    _std_simple_allocator Alloc>
// clang-format on
class basic_world<
    CompList<Comps...>, SystemLists, Observers, neutron::type_list<Locals...>,
    ResList<Reses...>, Alloc> : world_base<Alloc> {
    friend struct world_accessor;

    template <typename Ty>
    using _rebind_alloc_t = neutron::rebind_alloc_t<Alloc, Ty>;

public:
    using components   = CompList<Comps...>;
    using system_lists = SystemLists;
    using locals       = std::tuple<Locals...>;

    using archetype = archetype<Alloc>;

private:
    template <typename Ty>
    using allocator_t =
        typename std::allocator_traits<Alloc>::template rebind_alloc<Ty>;

    template <stage Stage>
    struct _get_systems {
        template <typename Ty>
        struct _is_specific_stage : std::false_type {};
        template <typename... Systems>
        struct _is_specific_stage<staged_type_list<Stage, Systems...>> :
            std::true_type {};
        using type = neutron::type_list_not_empty_t<
            staged_type_list<Stage>,
            neutron::type_list_first_t<
                neutron::type_list_filt_t<_is_specific_stage, system_lists>>>;
    };

    template <auto Sys>
    struct _call_system {
        using system_traits = _system_traits<decltype(Sys)>;
        using arg_list      = typename system_traits::arg_list;
        template <typename>
        struct call;
        template <template <typename...> typename Template, typename... Args>
        struct call<Template<Args...>> {
            void operator()(basic_world* world) noexcept(
                system_traits::is_nothrow) {
                Sys(make_from_world<Sys, Args>{}(*world)...);
            }
        };

        void operator()(basic_world* world) { call<arg_list>{}(world); }
    };

    template <typename>
    struct _call_system_list;
    template <template <auto...> typename Template, auto... Systems>
    struct _call_system_list<Template<Systems...>> {
        template <typename Scheduler>
        void operator()(Scheduler& scheduler, basic_world* self) {
            // TODO: execution
        }
    };

    template <typename>
    struct _call_run_list;
    template <stage Stage, typename... SysList>
    struct _call_run_list<staged_type_list<Stage, SysList...>> {
        template <typename Scheduler>
        void operator()(Scheduler& scheduler, basic_world* self) {
            (_call_system_list<SysList>{}(scheduler, self), ...);
        }
    };

public:
    basic_world(const Alloc& alloc = Alloc{})
        : world_base<Alloc>(alloc), locals_(), resources_() {}

    template <stage Stage, typename Scheduler>
    void call(Scheduler& scheduler) {
        using run_list = _get_systems<Stage>::type;
        _call_run_list<run_list>{}(scheduler, this);
        _apply_commands();
    }

private:
    /// variables could be use in only one specific system
    /// Locals are _sys_tuple, a tuple with system info, used to get the correct
    /// local for each sys
    neutron::shared_tuple<Locals...> locals_;
    // variables could be pass between each systems
    neutron::shared_tuple<Reses...> resources_;

    // constexpr static auto components_hash =
    // neutron::make_hash_array<components>();
};

struct world_accessor {
    template <_world World>
    static auto& archetypes(World& world) noexcept {
        return world.archetypes_;
    }
    template <_world World>
    static auto& entities(World& world) noexcept {
        return world.entities_;
    }
    template <_world World>
    static auto& locals(World& world) noexcept {
        return world.locals_;
    }
    template <_world World>
    static auto& resources(World& world) noexcept {
        return world.resources_;
    }
};

template <stage Stage, _world World, typename Scheduler>
void call(World& world, Scheduler& scheduler) {
    world.template call<Stage>(scheduler);
}

template <stage Stage, _world... Worlds, typename Scheduler>
void call(std::tuple<Worlds...>& worlds, Scheduler& scheduler) {
    [&worlds, &scheduler]<size_t... Is>(std::index_sequence<Is...>) {
        (std::get<Is>(worlds).template call<Stage>(scheduler), ...);
    }(std::index_sequence_for<Worlds...>());
}

template <_world... Worlds, typename Scheduler>
void call_startup(std::tuple<Worlds...>& worlds, Scheduler& scheduler) {
    call<stage::pre_startup>(worlds, scheduler);
    call<stage::startup>(worlds, scheduler);
    call<stage::post_startup>(worlds, scheduler);
}

template <_world... Worlds, typename Scheduler>
void call_update(std::tuple<Worlds...>& worlds, Scheduler& scheduler) {
    call<stage::pre_update>(worlds, scheduler);
    call<stage::update>(worlds, scheduler);
    call<stage::post_update>(worlds, scheduler);
}

template <_std_simple_allocator Alloc>
constexpr future_entity_t world_base<Alloc>::spawn() {
    return future_entity_t{ index_t{} };
}

template <_std_simple_allocator Alloc>
template <typename... Components>
constexpr future_entity_t world_base<Alloc>::spawn() {
    return future_entity_t{ index_t{} };
}

template <_std_simple_allocator Alloc>
template <typename... Components>
constexpr future_entity_t world_base<Alloc>::spawn(Components&&... components) {
    return future_entity_t{ index_t{} };
}

template <_std_simple_allocator Alloc>
template <typename... Components>
constexpr void world_base<Alloc>::add_components(entity_t entity) {
    //
}

template <_std_simple_allocator Alloc>
template <typename... Components>
constexpr void world_base<Alloc>::add_components(future_entity_t entity) {
    //
}

template <_std_simple_allocator Alloc>
template <typename... Components>
constexpr void world_base<Alloc>::remove_components(entity_t entity) {
    //
}

template <_std_simple_allocator Alloc>
template <typename... Components>
constexpr void world_base<Alloc>::remove_components(future_entity_t entity) {
    //
}

template <_std_simple_allocator Alloc>
constexpr void world_base<Alloc>::kill(entity_t entity) {
    //
}

template <_std_simple_allocator Alloc>
constexpr void world_base<Alloc>::kill(future_entity_t entity) {
    //
}

} // namespace proton
