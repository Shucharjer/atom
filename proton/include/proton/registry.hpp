#pragma once
#include <memory>
#include <type_traits>
#include <neutron/type_list.hpp>
#include "proton.hpp"
#include "proton/args/common/query.hpp"
#include "proton/args/system/local.hpp"
#include "proton/args/system/res.hpp"
#include "proton/observer.hpp"
#include "proton/stage.hpp"
#include "proton/system.hpp"

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

template <template <auto> typename, typename>
struct type_list_from_value;
template <template <auto> typename Predicate, template <auto...> typename Template, auto... Vals>
struct type_list_from_value<Predicate, Template<Vals...>> {
    using type = neutron::type_list<typename Predicate<Vals>::type...>;
};
template <template <auto> typename Predicate, typename ValList>
using type_list_from_value_t = typename type_list_from_value<Predicate, ValList>::type;

template <stage Stage, typename...>
struct staged_type_list {};
template <stage Stage, auto...>
struct staged_value_list {};
template <auto Sys, typename>
struct _sys_t {};

template <auto Desc>
struct _registry {
    using desc_type = std::remove_cvref_t<decltype(Desc)>;

    // value_list<auto...>
    using systems = parse_system_list<extract_systems_t<Desc>>;

    using system_list = neutron::type_list<
        staged_type_list<stage::pre_startup, typename systems::pre_startup_systems>,
        staged_type_list<stage::startup, typename systems::startup_systems>,
        staged_type_list<stage::post_startup, typename systems::post_startup_systems>,
        staged_type_list<stage::first, typename systems::first_systems>,
        staged_type_list<stage::pre_update, typename systems::pre_update_systems>,
        staged_type_list<stage::update, typename systems::update_systems>,
        staged_type_list<stage::post_update, typename systems::post_update_systems>,
        staged_type_list<stage::render, typename systems::render_systems>,
        staged_type_list<stage::last, typename systems::last_systems>,
        staged_type_list<stage::shutdown, typename systems::shutdown_systems>>;

    template <template <typename...> typename Template, auto Sys>
    struct _get_arg {
        using type = neutron::type_list_convert_t<
            std::remove_cvref, neutron::type_list_filt_nvoid_t<
                                   Template, typename _system_traits<decltype(Sys)>::arg_list>>;
    };

    // query<component...>
    template <auto Sys>
    using _get_qry = _get_arg<query, Sys>;
    using components =
        neutron::unique_type_list_t<neutron::type_list_first_t<neutron::type_list_not_empty_t<
            neutron::type_list<query<>>, neutron::type_list_list_cat_t<type_list_from_value_t<
                                             _get_qry, typename systems::all_systems>>>>>;

    // <on<?, ...>, ...>
    using observers = extract_observers_t<Desc>;

    // <<startup, <sys, ...>, <sys, ...>>, <update, ...>, ...>
    template <auto Sys>
    struct _get_loc {
        using type = _sys_t<
            Sys, neutron::type_list_filt_nvoid_t<
                     local, typename _system_traits<decltype(Sys)>::arg_list>>;
    };
    using locals = type_list_from_value_t<_get_loc, typename systems::all_systems>;
    // using locals = neutron::type_list_list_cat_t<type_list_from_value_t<_get_loc, typename
    // systems::all_systems>>;

    // <<startup, <sys, ...>, <sys, ...>>, <update, ...>, ...>
    template <auto Sys>
    using _get_res = _get_arg<res, Sys>;
    using resources =
        neutron::unique_type_list_t<neutron::type_list_first_t<neutron::type_list_list_cat_t<
            type_list_from_value_t<_get_res, typename systems::all_systems>>>>;
};

template <auto Desc>
struct registry {
    using systems     = _registry<Desc>::systems;
    using system_list = _registry<Desc>::system_list;
    using components  = _registry<Desc>::components;
    using observers   = _registry<Desc>::observers;
    using locals      = _registry<Desc>::locals;
    using resources   = _registry<Desc>::resources;
};

template <auto... WorldDesc, _std_simple_allocator Alloc = std::allocator<std::byte>>
auto make_worlds(const Alloc& alloc = Alloc{}) {
    // clang-format off
    return std::tuple<
        basic_world<
            typename registry<WorldDesc>::components,
            typename registry<WorldDesc>::system_list,
            typename registry<WorldDesc>::observers,
            typename registry<WorldDesc>::locals,
            typename registry<WorldDesc>::resources,
            Alloc
            >
        ...
    >();
    // clang-format on
}

} // namespace proton
