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
#include "proton/args/call_from_world.hpp"
#include "proton/command_buffer.hpp"
#include "proton/executor.hpp"
#include "proton/proton.hpp"
#include "proton/stage.hpp"
#include "proton/system.hpp"

namespace proton {

template <_std_simple_allocator Alloc>
class world_base {
    template <typename, typename, typename, typename, typename, _std_simple_allocator>
    friend class basic_world;

    template <typename Ty>
    using _rebind_alloc_t = neutron::rebind_alloc_t<Alloc, Ty>;

    using archetype = archetype<Alloc>;

public:
    template <typename Al = Alloc>
    constexpr world_base(const Al& alloc = Alloc{}) : archetypes_(alloc), entities_(alloc) {}

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
    neutron::shift_map<entity_t, index_t, 32UL, sizeof(entity_t) * 4UL, Alloc> entities_;
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
    CompList<Comps...>, SystemLists, Observers, neutron::type_list<Locals...>, ResList<Reses...>,
    Alloc> : world_base<Alloc> {
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
    using allocator_t = typename std::allocator_traits<Alloc>::template rebind_alloc<Ty>;

    template <stage Stage>
    struct _get_systems {
        template <typename Ty>
        struct _is_specific_stage : std::false_type {};
        template <typename... Systems>
        struct _is_specific_stage<staged_type_list<Stage, Systems...>> : std::true_type {};
        using type = neutron::type_list_not_empty_t<
            staged_type_list<Stage>, neutron::type_list_first_t<neutron::type_list_filt_t<
                                         _is_specific_stage, system_lists>>>;
    };

    template <auto Sys>
    struct _call_system {
        using system_traits = _system_traits<decltype(Sys)>;
        using arg_list      = typename system_traits::arg_list;
        template <typename>
        struct call;
        template <template <typename...> typename Template, typename... Args>
        struct call<Template<Args...>> {
            void operator()(basic_world* world) noexcept(system_traits::is_nothrow) {
                Sys(call_from_world<Sys, Args>{}(*world)...);
            }
        };

        void operator()(basic_world* world) { call<arg_list>{}(world); }
    };

    template <typename>
    struct _call_system_list;
    template <template <auto...> typename Template, auto... Systems>
    struct _call_system_list<Template<Systems...>> {
        template <typename Executor>
        void operator()(Executor& executor, basic_world* self) {
            executor
                .template submit<[](basic_world* self) { _call_system<Systems>{}(self); }...>(self)
                .wait();
        }
    };

    template <typename>
    struct _call_run_list;
    template <stage Stage, typename... SysList>
    struct _call_run_list<staged_type_list<Stage, SysList...>> {
        template <typename Executor>
        void operator()(Executor& executor, basic_world* self) {
            (_call_system_list<SysList>{}(executor, self), ...);
        }
    };

public:
    basic_world(const Alloc& alloc = Alloc{}) : world_base<Alloc>(alloc), locals_(), resources_() {}

    template <stage Stage, typename Executor = single_task_executor>
    void call(const Executor& executor = Executor{}) {
        using run_list = _get_systems<Stage>::type;
        _call_run_list<run_list>{}(executor, this);
    }

    template <stage Stage, typename Executor = single_task_executor>
    void call(Executor& executor) {
        using run_list = _get_systems<Stage>::type;
        _call_run_list<run_list>{}(executor, this);
    }

private:
    /// variables could be use in only one specific system
    /// Locals are _sys_tuple, a tuple with system info, used to get the correct local for each sys
    std::tuple<Locals...> locals_;
    // variables could be pass between each systems
    neutron::shared_tuple<Reses...> resources_;

    // constexpr static auto components_hash = neutron::make_hash_array<components>();
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

template <stage Stage, _world World, typename Executor = single_task_executor>
void call(World& world, const Executor& executor = Executor{}) {
    world.template call<Stage>(executor);
}

template <stage Stage, _world World, typename Executor>
void call(World& world, Executor& executor) {
    world.template call<Stage>(executor);
}

template <stage Stage, _world... Worlds, typename Executor = single_task_executor>
void call(std::tuple<Worlds...>& worlds, const Executor& executor = Executor{}) {
    [&worlds, &executor]<size_t... Is>(std::index_sequence<Is...>) {
        (std::get<Is>(worlds).template call<Stage>(executor), ...);
    }(std::index_sequence_for<Worlds...>());
}

template <stage Stage, _world... Worlds, typename Executor>
void call(std::tuple<Worlds...>& worlds, Executor& executor) {
    [&worlds, &executor]<size_t... Is>(std::index_sequence<Is...>) {
        (std::get<Is>(worlds).template call<Stage>(executor), ...);
    }(std::index_sequence_for<Worlds...>());
}

template <_world... Worlds, typename Executor = single_task_executor>
void call_startup(std::tuple<Worlds...>& worlds, const Executor& executor = Executor{}) {
    call<stage::pre_startup>(worlds, executor);
    call<stage::startup>(worlds, executor);
    call<stage::post_startup>(worlds, executor);
}

template <_world... Worlds, typename Executor>
void call_startup(std::tuple<Worlds...>& worlds, Executor& executor) {
    call<stage::pre_startup>(worlds, executor);
    call<stage::startup>(worlds, executor);
    call<stage::post_startup>(worlds, executor);
}

template <_world... Worlds, typename Executor = single_task_executor>
void call_update(std::tuple<Worlds...>& worlds, const Executor& executor = Executor{}) {
    call<stage::pre_update>(worlds, executor);
    call<stage::update>(worlds, executor);
    call<stage::post_update>(worlds, executor);
}

template <_world... Worlds, typename Executor>
void call_update(std::tuple<Worlds...>& worlds, Executor& executor) {
    call<stage::pre_update>(worlds, executor);
    call<stage::update>(worlds, executor);
    call<stage::post_update>(worlds, executor);
}

} // namespace proton
