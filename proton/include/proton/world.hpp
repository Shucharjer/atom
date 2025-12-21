#pragma once
#include "proton/proton.hpp"

#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>
#include <neutron/execution.hpp>
#include <neutron/memory.hpp>
#include <neutron/shared_tuple.hpp>
#include <neutron/type_hash.hpp>
#include "proton/command_buffer.hpp"
#include "proton/registry.hpp"
#include "proton/stage.hpp"
#include "proton/system.hpp"
#include "proton/world_base.hpp"

namespace proton {

template <_std_simple_allocator Alloc>
class basic_world<registry<world_desc>, Alloc> : world_base<Alloc> {
    template <auto, typename, size_t>
    friend struct construct_from_world_t;
    friend struct world_accessor;

    template <typename Ty>
    using _allocator_t = neutron::rebind_alloc_t<Alloc, Ty>;

    template <typename Ty>
    using _vector_t = std::vector<Ty, _allocator_t<Ty>>;

public:
    using allocator_type = Alloc;
    using registry_t     = registry<world_desc>;
    using components     = typename registry_t::components;
    using resources      = typename registry_t::resources;
    using system_lists   = typename registry_t::system_list;
    using systems        = typename registry_t::systems;
    using locals         = typename registry_t::locals;
    using archetype      = typename world_base<Alloc>::archetype;
    using command_buffer = command_buffer<Alloc>;

    template <typename Al = Alloc>
    constexpr explicit basic_world(const Al& alloc = {})
        : world_base<Alloc>(alloc) {}

private:
    _vector_t<command_buffer>* command_buffers_;
};

template <typename Registry, _std_simple_allocator Alloc>
class basic_world : world_base<Alloc> {
    template <auto, typename, size_t>
    friend struct construct_from_world_t;
    friend struct world_accessor;

    auto _base() & noexcept -> world_base<Alloc>& {
        return *static_cast<world_base<Alloc>*>(this);
    }

    auto _base() const& noexcept -> const world_base<Alloc>& {
        return *static_cast<const world_base<Alloc>*>(this);
    }

    template <typename Ty>
    using _allocator_t = neutron::rebind_alloc_t<Alloc, Ty>;

    template <typename Ty>
    using _vector_t = std::vector<Ty, _allocator_t<Ty>>;

public:
    using allocator_type = Alloc;
    using registry_t     = Registry;
    using components     = typename registry_t::components;
    using resources      = typename registry_t::resources;
    using system_lists   = typename registry_t::system_list;
    using systems        = typename registry_t::systems;
    using locals         = typename registry_t::locals;
    using archetype      = typename world_base<Alloc>::archetype;
    using command_buffer = command_buffer<Alloc>;

    template <typename Al = Alloc>
    constexpr explicit basic_world(const Al& alloc = {})
        : world_base<Alloc>(alloc), resources_(), locals_() {}

    template <stage Stage, neutron::execution::scheduler Sch>
    void call(Sch& sch, _vector_t<command_buffer>& cmdbufs) {
        using run_list   = _get_systems<Stage>::type;
        command_buffers_ = &cmdbufs;
        _call_run_list<run_list>{}(sch, this);
    }

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

    // call a system with corresponding arguments
    template <size_t Index, auto Sys>
    struct _call_sys {
        using system_traits = _system_traits<decltype(Sys)>;
        using arg_list      = typename system_traits::arg_list;
        template <typename>
        struct call;
        template <template <typename...> typename Template, typename... Args>
        struct call<Template<Args...>> {
            void operator()(basic_world* world) const
                noexcept(system_traits::is_nothrow) {
                Sys(construct_from_world<Sys, Args, Index>(*world)...);
            }
        };

        void operator()(basic_world* world) { call<arg_list>{}(world); }
    };

    void _apply_command_buffers() noexcept {
        for (auto& cmdbuf : *command_buffers_) {
            cmdbuf.apply(_base());
        }
    }

    // call systems in the same system list.
    template <typename>
    struct _call;
    template <template <auto...> typename Template, auto... Systems>
    struct _call<Template<Systems...>> {
        template <typename Sch>
        void operator()(Sch& sch, basic_world* self) const noexcept {
            using namespace neutron;
            using namespace neutron::execution;
            using slist = value_list<Systems...>;

            for (auto& cmdbuf : *self->command_buffers_) {
                cmdbuf.reset();
            }

            auto all = [&sch, self]<size_t... Is>(std::index_sequence<Is...>) {
                return when_all(
                    (schedule(sch) | then([self] {
                         _call_sys<Is, value_list_element_v<Is, slist>>{}(self);
                     }))...);
            }(std::index_sequence_for<value_list<Systems>...>());
            sync_wait(std::move(all));
            self->_apply_command_buffers();
        }
    };

    // call system lists
    template <typename>
    struct _call_run_list;
    template <stage Stage, typename... SysList>
    struct _call_run_list<staged_type_list<Stage, SysList...>> {
        template <neutron::execution::scheduler Sch>
        void operator()(Sch& sch, basic_world* self) {
            using namespace neutron;
            using namespace neutron::execution;

            using syslist = type_list<SysList...>;
            [&]<size_t... Is>(std::index_sequence<Is...>) {
                (_call<type_list_element_t<Is, syslist>>{}(sch, self), ...);
            }(std::index_sequence_for<SysList...>());
        }
    };

    /// variables could be use in only one specific system
    /// Locals are _sys_tuple, a tuple with system info, used to get the correct
    /// local for each sys
    neutron::type_list_rebind_t<std::tuple, locals> locals_;
    // variables could be pass between each systems
    neutron::type_list_rebind_t<neutron::shared_tuple, resources> resources_;

    _vector_t<command_buffer>* command_buffers_;
};

template <
    stage Stage, neutron::execution::scheduler Sch, world World,
    typename CmdBuf                     = World::command_buffer,
    template <typename> typename Vector = World::template _vector_t>
void call(Sch& sch, Vector<CmdBuf>& cmdbufs, World& world) {
    world.template call<Stage>(sch, cmdbufs);
}

template <
    stage Stage, neutron::execution::scheduler Sch, typename Alloc,
    world... Worlds,
    typename VectorAlloc =
        neutron::rebind_alloc_t<Alloc, command_buffer<Alloc>>>
void call(
    Sch& sch, std::vector<command_buffer<Alloc>, VectorAlloc>& cmdbufs,
    std::tuple<Worlds...>& worlds) {
    [&]<size_t... Is>(std::index_sequence<Is...>) {
        (std::get<Is>(worlds).template call<Stage>(sch, cmdbufs), ...);
    }(std::index_sequence_for<Worlds...>());
}

void call_startup(
    neutron::execution::scheduler auto& sch, auto& cmdbufs, world auto& world) {
    call<stage::pre_startup>(sch, cmdbufs, world);
    call<stage::startup>(sch, cmdbufs, world);
    call<stage::post_startup>(sch, cmdbufs, world);
}

template <world... Worlds>
void call_startup(
    neutron::execution::scheduler auto& sch, auto& cmdbufs,
    std::tuple<Worlds...>& worlds) {
    call<stage::pre_startup>(sch, cmdbufs, worlds);
    call<stage::startup>(sch, cmdbufs, worlds);
    call<stage::post_startup>(sch, cmdbufs, worlds);
}

void call_update(
    neutron::execution::scheduler auto& sch, auto& cmdbufs, world auto& world) {
    call<stage::pre_update>(sch, cmdbufs, world);
    call<stage::update>(sch, cmdbufs, world);
    call<stage::post_update>(sch, cmdbufs, world);
}

template <world... Worlds>
void call_update(
    neutron::execution::scheduler auto& sch, auto& cmdbufs,
    std::tuple<Worlds...>& worlds) {
    call<stage::pre_update>(sch, cmdbufs, worlds);
    call<stage::update>(sch, cmdbufs, worlds);
    call<stage::post_update>(sch, cmdbufs, worlds);
}

} // namespace proton
