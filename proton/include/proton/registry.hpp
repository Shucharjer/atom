#pragma once
#include <memory>
#include <type_traits>
#include <neutron/template_list.hpp>
#include "proton.hpp"
#include "proton/args/common/query.hpp"
#include "proton/args/system/local.hpp"
#include "proton/args/system/res.hpp"
#include "proton/observer.hpp"
#include "proton/stage.hpp"
#include "proton/system.hpp"

namespace proton {

template <auto Sys, typename>
struct _sys_t {};

template <auto Obj>
struct _get_type {
    using type = std::remove_cvref_t<decltype(Obj)>;
};
template <auto Obj>
using _get_type_t = typename _get_type<Obj>::type;

template <auto Desc>
struct _registry {
    using desc_type = _get_type<Desc>;

    // value_list<auto...>
    using systems = parse_system_list<extract_systems_t<Desc>>;

    using system_list = neutron::type_list<
        staged_type_list_from_value_t<
            stage::pre_startup, typename systems::parsed_pre_startup_systems>,
        staged_type_list_from_value_t<stage::startup, typename systems::parsed_startup_systems>,
        staged_type_list_from_value_t<
            stage::post_startup, typename systems::parsed_post_startup_systems>,
        staged_type_list_from_value_t<stage::first, typename systems::parsed_first_systems>,
        staged_type_list_from_value_t<
            stage::pre_update, typename systems::parsed_pre_update_systems>,
        staged_type_list_from_value_t<stage::update, typename systems::parsed_update_systems>,
        staged_type_list_from_value_t<
            stage::post_update, typename systems::parsed_post_update_systems>,
        staged_type_list_from_value_t<stage::render, typename systems::parsed_render_systems>,
        staged_type_list_from_value_t<stage::last, typename systems::parsed_last_systems>,
        staged_type_list_from_value_t<stage::shutdown, typename systems::parsed_shutdown_systems>>;

    template <template <typename...> typename Template, auto Sys>
    struct _get_tlist {
        using type = neutron::type_list_filt_type_list_t<
            Template, typename _system_traits<decltype(Sys)>::arg_list>;
    };

    template <template <auto...> typename Template, auto Sys>
    struct _get_vlist {
        using type = neutron::type_list_filt_value_list_t<
            Template, typename _system_traits<decltype(Sys)>::arg_list>;
    };

    // query<with_objs...>
    template <auto Sys>
    using _get_qry = _get_tlist<query, Sys>;
    // using querys   = neutron::type_list_first_t<
    //       neutron::type_list_value_list_cat_t<neutron::type_list_not_empty_t<
    //           neutron::type_list<query<>>,
    //           neutron::type_list_from_value_t<_get_qry, typename systems::all_systems>>>>;
    using querys = neutron::type_list_from_value_t<_get_qry, typename systems::all_systems>;

    using components = std::remove_pointer_t<decltype([] {
        using namespace neutron;
        using querys            = type_list_filt_type_list_t<query, querys>;
        using combined_t        = type_list_conbine_t<querys>;
        using with_like_only_t  = type_list_filt_t<_is_with_like, combined_t>;
        using exposed_with_t    = type_list_expose_t<with, with_like_only_t>;
        using exposed_without_t = type_list_expose_t<without, exposed_with_t>;
        using exposed_withany_t = type_list_expose_t<withany, exposed_without_t>;
        using removed_cvref_t   = type_list_convert_t<std::remove_cvref, exposed_withany_t>;
        using exposed_bundle_t  = type_list_recurse_expose_t<bundle, removed_cvref_t>;
        using unique_type_t     = unique_type_list_t<exposed_bundle_t>;
        using type              = unique_type_t;
        return static_cast<type*>(nullptr);
    }())>;

    // <on<?, ...>, ...>
    using observers = extract_observers_t<Desc>;

    // <<sys, ...>, <sys, ...>>, <sys, ...>, ...>
    template <auto Sys>
    struct _get_loc {
        using type = neutron::type_list_with_value_t<
            _sys_tuple, Sys,
            neutron::type_list_filt_type_list_t<
                local, typename _system_traits<decltype(Sys)>::arg_list>>;
    };
    template <typename>
    struct filt_fn;
    template <template <auto, typename...> typename Template, auto Sys, typename... Args>
    struct filt_fn<Template<Sys, Args...>> {
        constexpr static bool value = sizeof...(Args) != 0;
    };
    using locals = neutron::type_list_filt_t<
        filt_fn, neutron::type_list_from_value_t<_get_loc, typename systems::all_systems>>;

    // <<startup, <sys, ...>, <sys, ...>>, <update, ...>, ...>
    template <auto Sys>
    using _get_res = _get_tlist<res, Sys>;
    using resources =
        neutron::type_list_convert_t<std::remove_cvref, neutron::unique_type_list_t<neutron::type_list_first_t<neutron::type_list_not_empty_t<
            neutron::type_list<res<>>,
            neutron::type_list_list_cat_t<
                neutron::type_list_from_value_t<_get_res, typename systems::all_systems>>>>>>;
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
