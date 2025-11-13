#pragma once
#include "neutron/template_list.hpp"

namespace proton {

template <typename... Args>
struct with {};

template <typename... Args>
struct without {};

template <typename... Args>
struct withany {};

template <typename Ty>
struct _is_with_like {
    constexpr static auto value = neutron::is_specific_type_list_v<with, Ty> ||
                                  neutron::is_specific_type_list_v<without, Ty> ||
                                  neutron::is_specific_type_list_v<withany, Ty>;
};

template <typename Ty>
struct _is_with : neutron::is_specific_type_list<with, Ty> {};
template <typename Ty>
constexpr auto _is_with_v = _is_with<Ty>::value;

template <typename Ty>
struct _is_without : neutron::is_specific_type_list<without, Ty> {};
template <typename Ty>
constexpr auto _is_without_v = _is_without<Ty>::value;

template <typename Ty>
struct _is_with_any : neutron::is_specific_type_list<withany, Ty> {};
template <typename Ty>
constexpr auto _is_with_any_v = _is_with_any<Ty>::value;

template <typename... Args>
consteval bool _with_obj_assert() {
    using namespace neutron;

    using type_list    = neutron::type_list<Args...>;
    using with_list    = type_list_filt_type_list_t<with, type_list>;
    using without_list = type_list_filt_type_list_t<without, type_list>;
    using withany_list = type_list_filt_type_list_t<withany, type_list>;

    return type_list_all_differs_from_v<with_list, without_list> &&
           type_list_all_differs_from_v<with_list, withany_list> &&
           type_list_all_differs_from_v<without_list, withany_list>;
}

} // namespace proton
