#pragma once
#include <type_traits>
#include <neutron/type_list.hpp>
#include "proton/args/common/query.hpp"
#include "proton/system.hpp"
#include "proton/world_desc.hpp"

namespace proton {

template <typename>
struct _system_traits;
template <typename Ret, typename... Args>
struct _system_traits<Ret (*)(Args...)> {
    using arg_list                   = neutron::type_list<Args...>;
    constexpr static bool is_nothrow = false;
};
template <typename Ret, typename... Args>
struct _system_traits<Ret (*)(Args...) noexcept> {
    using arg_list                   = neutron::type_list<Args...>;
    constexpr static bool is_nothrow = true;
};

template <typename>
struct _registry;
template <typename... Descriptors>
struct _registry<world_descriptor<Descriptors...>> {
    using world_desc = world_descriptor<Descriptors...>;
    using parsed_systems =
        parse_system_list<neutron::type_list_filt_nvoid_t<system_list, world_desc>>;
    using all_systems = typename parsed_systems::all_systems;
    template <typename>
    struct _to_system_arg_list;
    template <auto... Sys>
    struct _to_system_arg_list<neutron::value_list<Sys...>> {
        using type = neutron::type_list_cat_t<typename _system_traits<decltype(Sys)>::arg_list...>;
    };
    using components = typename _to_system_arg_list<all_systems>::type;
};

template <auto Desc>
struct registry {
    using desc_type     = std::remove_cvref_t<decltype(Desc)>;
    using registry_type = _registry<desc_type>;
    // using components    = typename registry_type::components;
};

} // namespace proton
