#pragma once
#include <type_traits>
#include <neutron/pipeline.hpp>
#include <neutron/type_list.hpp>
#include <neutron/value_list.hpp>
#include "proton/desc_list.hpp"
#include "proton/stage.hpp"

namespace proton {

template <typename...>
struct system_list {};

template <stage Stage, auto System, typename... Requires>
struct _system {};

template <stage Stage, auto System, typename... Requires>
constexpr inline add_staged_fn<system_list, _system, Stage, System, Requires...> add_system;

template <auto... Systems>
struct before {};

template <auto... Systems>
struct after {};

namespace internal {

template <typename>
struct add_system_traits;
template <stage Stage, auto System, typename... Requires>
struct add_system_traits<_system<Stage, System, Requires...>> {
    constexpr static auto stage     = Stage;
    constexpr static auto system_fn = System;
    using requirements              = neutron::type_list<Requires...>;
};

template <typename...>
struct run_list {};

template <typename Context, typename System>
struct _same_stage_is_before {};
template <
    stage Stage, auto ContextSystem, typename... ContextRequires, auto System, typename... Requires>
struct _same_stage_is_before<
    _system<Stage, ContextSystem, ContextRequires...>, _system<Stage, System, Requires...>> {

    template <typename Require>
    using the_requirement_is_before = neutron::is_specific_value_list<before, Require>;

    using filted_type =
        neutron::type_list_filt_t<the_requirement_is_before, neutron::type_list<Requires...>>;

    template <typename>
    struct _contains_system : std::false_type {};
    template <auto... Systems>
    struct _contains_system<before<Systems...>> {
        constexpr static bool value = (neutron::is_same_value_v<ContextSystem, Systems> || ...);
    };

    constexpr static bool value = [] {
        if constexpr (neutron::is_empty_template_v<filted_type>) {
            return false;
        } else {
            using before_list = neutron::type_list_first_t<filted_type>;
            return _contains_system<before_list>::value;
        }
    }();
};
template <typename Context, typename System>
constexpr auto _same_stage_is_before_v = _same_stage_is_before<Context, System>::value;

template <typename Context, typename Ty>
struct _same_stage_is_after {};
template <
    stage Stage, auto ContextSystem, typename... ContextRequires, auto System, typename... Requires>
struct _same_stage_is_after<
    _system<Stage, ContextSystem, ContextRequires...>, _system<Stage, System, Requires...>> {

    template <typename Require>
    using the_requirement_is_after = neutron::is_specific_value_list<after, Require>;

    using filted_type =
        neutron::type_list_filt_t<the_requirement_is_after, neutron::type_list<Requires...>>;

    template <typename>
    struct _contains_system : std::false_type {};
    template <auto... Systems>
    struct _contains_system<after<Systems...>> {
        constexpr static bool value = (neutron::is_same_value_v<ContextSystem, Systems> || ...);
    };

    constexpr static bool value = [] {
        if constexpr (neutron::is_empty_template_v<filted_type>) {
            return false;
        } else {
            using after_list = neutron::type_list_first_t<filted_type>;
            return _contains_system<after_list>::value;
        }
    }();
};
template <typename Context, typename AddSysFn>
constexpr auto _same_stage_is_after_v = _same_stage_is_after<Context, AddSysFn>::value;

template <typename AddSysFn1, typename AddSysFn2>
struct _same_stage_is_available;
template <stage Stage, auto Sys1, auto Sys2, typename... Requires1, typename... Requires2>
struct _same_stage_is_available<
    _system<Stage, Sys1, Requires1...>, _system<Stage, Sys2, Requires2...>> {
    using sys1 = _system<Stage, Sys1, Requires1...>;
    using sys2 = _system<Stage, Sys2, Requires2...>;
    constexpr static bool value =
        !((_same_stage_is_before_v<sys1, sys2> && _same_stage_is_after_v<sys1, sys2>) ||
          (_same_stage_is_before_v<sys2, sys1> && _same_stage_is_after_v<sys2, sys1>) ||
          (_same_stage_is_before_v<sys1, sys2> && _same_stage_is_after_v<sys2, sys1>) ||
          (_same_stage_is_before_v<sys2, sys1> && _same_stage_is_after_v<sys1, sys2>));
};
template <typename AddSysFn1, typename AddSysFn2>
constexpr auto _same_stage_is_available_v = _same_stage_is_available<AddSysFn1, AddSysFn2>::value;

template <typename, typename System>
struct _same_stage_system_check_one;
template <typename... Systems, typename System>
struct _same_stage_system_check_one<system_list<Systems...>, System> {
    constexpr static bool value = (_same_stage_is_available_v<Systems, System> && ...);
};

template <typename>
struct same_stage_system_check;
template <typename... Systems>
struct same_stage_system_check<system_list<Systems...>> {
    static_assert((_same_stage_system_check_one<system_list<Systems...>, Systems>::value && ...));
};

template <typename>
struct parse_same_stage_system_list;

template <typename>
struct extract_systems;
template <typename... AddSys>
struct extract_systems<system_list<AddSys...>> {
    using type = neutron::value_list<add_system_traits<AddSys>::system_fn...>;
};
template <typename SysList>
using extract_systems_t = typename extract_systems<SysList>::type;

template <typename... Systems>
struct parse_same_stage_system_list<system_list<Systems...>> {
    using check_type = same_stage_system_check<system_list<Systems...>>;

    template <typename, typename>
    struct in_degree_is_zero;
    template <typename Sys>
    struct in_degree_is_zero<system_list<>, Sys> : std::true_type {};
    template <typename... S, typename Sys>
    struct in_degree_is_zero<system_list<S...>, Sys> {
        constexpr static bool value =
            !(_same_stage_is_before_v<Sys, S> || ...) && !(_same_stage_is_after_v<S, Sys> || ...);
    };

    template <typename Curr, typename Remains>
    struct parse;
    template <typename... SysLists>
    struct parse<run_list<SysLists...>, system_list<>> {
        using type = run_list<SysLists...>;
    };
    template <typename... SysLists, typename... Remains>
    struct parse<run_list<SysLists...>, system_list<Remains...>> {

        template <typename Curr, typename List>
        struct _in_degree_is_zero;
        template <typename... Sys>
        struct _in_degree_is_zero<system_list<Sys...>, system_list<>> {
            using type = system_list<Sys...>;
        };
        template <typename... Sys, typename Ty, typename... Others>
        struct _in_degree_is_zero<system_list<Sys...>, system_list<Ty, Others...>> {
            using current_type = std::conditional_t<
                in_degree_is_zero<system_list<Remains...>, Ty>::value, system_list<Sys..., Ty>,
                system_list<Sys...>>;
            using type = typename _in_degree_is_zero<current_type, system_list<Others...>>::type;
        };

        using filted_type =
            typename _in_degree_is_zero<system_list<>, system_list<Remains...>>::type;
        using removed_type = neutron::type_list_remove_in_t<filted_type, system_list<Remains...>>;
        using type         = typename parse<run_list<SysLists..., filted_type>, removed_type>::type;
    };

    using parsed_type = typename parse<run_list<>, system_list<Systems...>>::type;

    template <typename>
    struct _extract;
    template <typename... SysLists>
    struct _extract<run_list<SysLists...>> {
        using type = run_list<extract_systems_t<SysLists>...>;
    };

    using type = typename _extract<parsed_type>::type;
};

} // namespace internal

template <auto WorldDesc>
using extract_systems_t = neutron::type_list_filt_nvoid_t<system_list, desc_t<WorldDesc>>;

template <typename>
struct parse_system_list;
template <typename... Systems>
struct parse_system_list<system_list<Systems...>> {
    using systems = system_list<Systems...>;

    // original, keep requires

    using _pre_startup_systems  = neutron::type_list_filt_t<in_pre_startup_stage, systems>;
    using _startup_systems      = neutron::type_list_filt_t<in_startup_stage, systems>;
    using _post_startup_systems = neutron::type_list_filt_t<in_post_startup_stage, systems>;

    using _first_systems = neutron::type_list_filt_t<in_first_stage, systems>;

    using _pre_update_systems  = neutron::type_list_filt_t<in_pre_update_stage, systems>;
    using _update_systems      = neutron::type_list_filt_t<in_update_stage, systems>;
    using _post_update_systems = neutron::type_list_filt_t<in_post_update_stage, systems>;

    using _last_systems = neutron::type_list_filt_t<in_last_stage, systems>;

    using _shutdown_systems = neutron::type_list_filt_t<in_shutdown_stage, systems>;

    // extract, for further meta info

    using all_systems          = internal::extract_systems_t<systems>;
    using pre_startup_systems  = internal::extract_systems_t<_pre_startup_systems>;
    using startup_systems      = internal::extract_systems_t<_startup_systems>;
    using post_startup_systems = internal::extract_systems_t<_post_startup_systems>;
    using first_systems        = internal::extract_systems_t<_first_systems>;
    using pre_update_systems   = internal::extract_systems_t<_pre_update_systems>;
    using update_systems       = internal::extract_systems_t<_update_systems>;
    using post_update_systems  = internal::extract_systems_t<_post_update_systems>;
    using last_systems         = internal::extract_systems_t<_last_systems>;
    using shutdown_systems     = internal::extract_systems_t<_shutdown_systems>;

    // parsed, each one is a run_list

    using parsed_pre_startup_systems =
        typename internal::parse_same_stage_system_list<_pre_startup_systems>::type;
    using parsed_startup_systems =
        typename internal::parse_same_stage_system_list<_startup_systems>::type;
    using parsed_post_startup_systems =
        typename internal::parse_same_stage_system_list<_post_startup_systems>::type;

    using parsed_first_systems =
        typename internal::parse_same_stage_system_list<_first_systems>::type;

    using parsed_pre_update_systems =
        typename internal::parse_same_stage_system_list<_pre_update_systems>::type;
    using parsed_update_systems =
        typename internal::parse_same_stage_system_list<_update_systems>::type;
    using parsed_post_update_systems =
        typename internal::parse_same_stage_system_list<_post_update_systems>::type;

    using parsed_last_systems =
        typename internal::parse_same_stage_system_list<_last_systems>::type;

    using parsed_shutdown_systems =
        typename internal::parse_same_stage_system_list<_shutdown_systems>::type;
};

} // namespace proton
