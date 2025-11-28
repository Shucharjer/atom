#pragma once
#include "proton/proton.hpp"

#include <cstddef>
#include <type_traits>
#include <utility>
#include <neutron/memory.hpp>
#include <neutron/shared_tuple.hpp>
#include <neutron/shift_map.hpp>
#include <neutron/type_hash.hpp>
#include "proton.hpp"
#include "proton/registry.hpp"
#include "proton/stage.hpp"
#include "proton/system.hpp"
#include "proton/world_base.hpp"

namespace proton {

template <_std_simple_allocator Alloc>
class basic_world<registry<world_desc>, Alloc> : world_base<Alloc> {
    friend struct world_accessor;

public:
    using registry_t   = registry<world_desc>;
    using components   = typename registry_t::components;
    using resources    = typename registry_t::resources;
    using system_lists = typename registry_t::system_list;
    using systems      = typename registry_t::systems;
    using locals       = typename registry_t::locals;
    using archetype    = typename world_base<Alloc>::archetype;

    template <typename Al = Alloc>
    constexpr explicit basic_world(const Al& alloc = {})
        : world_base<Alloc>(alloc) {}
};

template <typename Registry, _std_simple_allocator Alloc>
class basic_world : world_base<Alloc> {
    friend struct world_accessor;

    template <typename Ty>
    using _allocator_t = neutron::rebind_alloc_t<Alloc, Ty>;

public:
    using registry_t   = Registry;
    using components   = typename registry_t::components;
    using resources    = typename registry_t::resources;
    using system_lists = typename registry_t::system_list;
    using systems      = typename registry_t::systems;
    using locals       = typename registry_t::locals;
    using archetype    = typename world_base<Alloc>::archetype;

private:
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
                Sys(construct_from_world<Sys, Args>(*world)...);
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
    template <typename Al = Alloc>
    constexpr explicit basic_world(const Al& alloc = {})
        : world_base<Alloc>(alloc), resources_(), locals_() {}

    template <stage Stage, typename Scheduler>
    void call(Scheduler& scheduler) {
        using run_list = _get_systems<Stage>::type;
        _call_run_list<run_list>{}(scheduler, this);
        // _apply_commands();
    }

private:
    /// variables could be use in only one specific system
    /// Locals are _sys_tuple, a tuple with system info, used to get the correct
    /// local for each sys
    neutron::type_list_rebind_t<std::tuple, locals> locals_;
    // variables could be pass between each systems
    neutron::type_list_rebind_t<neutron::shared_tuple, resources> resources_;

    // constexpr static auto components_hash =
    // neutron::make_hash_array<components>();
};

template <stage Stage, world World, typename Scheduler>
void call(World& world, Scheduler& scheduler) {
    world.template call<Stage>(scheduler);
}

template <stage Stage, world... Worlds, typename Scheduler>
void call(std::tuple<Worlds...>& worlds, Scheduler& scheduler) {
    [&worlds, &scheduler]<size_t... Is>(std::index_sequence<Is...>) {
        (std::get<Is>(worlds).template call<Stage>(scheduler), ...);
    }(std::index_sequence_for<Worlds...>());
}

template <world... Worlds, typename Scheduler>
void call_startup(std::tuple<Worlds...>& worlds, Scheduler& scheduler) {
    call<stage::pre_startup>(worlds, scheduler);
    call<stage::startup>(worlds, scheduler);
    call<stage::post_startup>(worlds, scheduler);
}

template <world... Worlds, typename Scheduler>
void call_update(std::tuple<Worlds...>& worlds, Scheduler& scheduler) {
    call<stage::pre_update>(worlds, scheduler);
    call<stage::update>(worlds, scheduler);
    call<stage::post_update>(worlds, scheduler);
}

} // namespace proton
