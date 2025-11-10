#pragma once
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include <neutron/shift_map.hpp>
#include <neutron/type_hash.hpp>
#include "args/common/command_buffer.hpp"
#include "proton.hpp"
#include "proton/archetype.hpp"
#include "proton/executor.hpp"
#include "proton/proton.hpp"
#include "proton/stage.hpp"
#include "proton/system.hpp"

namespace proton {

using entity_t     = uint64_t;
using generation_t = uint32_t;
using index_t      = uint32_t;

// clang-format off
template <
    template <typename...> typename CompList, _comp_or_bundle... Comps,
    typename... PreStartupSystems,
    typename... StartupSystems,
    typename... PostStartupSystems,
    typename... FirstSystems,
    typename... PreUpdateSystems,
    typename... UpdateSystems,
    typename... PostUpdateSystems,
    typename... RenderSystems,
    typename... LastSystems,
    typename... ShutdownSystems,
    typename Observers,
    typename... Locals,
    template <typename...> typename ResList, _res_or_bundle... Reses,
    _std_simple_allocator Alloc>
// clang-format on
class basic_world<
    CompList<Comps...>,
    neutron::type_list<
        // staged_type_list<stage, Sys...>
        staged_type_list<stage::pre_startup, PreStartupSystems...>,
        staged_type_list<stage::startup, StartupSystems...>,
        staged_type_list<stage::post_startup, PostStartupSystems...>,
        staged_type_list<stage::first, FirstSystems...>,
        staged_type_list<stage::pre_update, PreUpdateSystems...>,
        staged_type_list<stage::update, UpdateSystems...>,
        staged_type_list<stage::post_update, PostUpdateSystems...>,
        staged_type_list<stage::render, RenderSystems...>,
        staged_type_list<stage::last, LastSystems...>,
        staged_type_list<stage::shutdown, ShutdownSystems...>>,
    Observers, neutron::type_list<Locals...>, ResList<Reses...>, Alloc> {
    friend struct world_accessor;

    using system_list = neutron::type_list<
        // staged_type_list<stage, Sys...>
        staged_type_list<stage::pre_startup, PreStartupSystems...>,
        staged_type_list<stage::startup, StartupSystems...>,
        staged_type_list<stage::post_startup, PostStartupSystems...>,
        staged_type_list<stage::first, FirstSystems...>,
        staged_type_list<stage::pre_update, PreUpdateSystems...>,
        staged_type_list<stage::update, UpdateSystems...>,
        staged_type_list<stage::post_update, PostUpdateSystems...>,
        staged_type_list<stage::render, RenderSystems...>,
        staged_type_list<stage::last, LastSystems...>,
        staged_type_list<stage::shutdown, ShutdownSystems...>>;

    template <typename Ty>
    using allocator_t = typename std::allocator_traits<Alloc>::template rebind_alloc<Ty>;

    template <stage Stage>
    struct _get_systems {
        template <typename Ty>
        struct _is_specific_stage : std::false_type {};
        template <typename... Systems>
        struct _is_specific_stage<staged_type_list<Stage, Systems...>> : std::true_type {};
        using type = neutron::type_list_not_empty_t<
            staged_type_list<Stage>,
            neutron::type_list_first_t<neutron::type_list_filt_t<_is_specific_stage, system_list>>>;
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
                Sys(Args{ *world }...);
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
    using components = CompList<Comps...>;

    basic_world(const Alloc& alloc = Alloc{}) : archetypes_(alloc), entities_(alloc), locals_() {}

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
    std::vector<archetype, allocator_t<archetype>> archetypes_;
    // mapping entity to the archetype stores it
    neutron::shift_map<entity_t, id_t, Alloc> entities_;
    // command buffer
    basic_command_buffer<Alloc> command_buffer_;
    // variables could be use in only one specific system
    std::tuple<Locals...> locals_;
    // variables could be pass between each systems
    std::tuple<Reses...> resources_;

    // constexpr static auto components_hash = neutron::make_hash_array<components>();
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
    static auto& resources(World& world) noexcept {
        return world.resources_;
    }
};

} // namespace proton
