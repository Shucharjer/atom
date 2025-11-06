#pragma once
#include <cstddef>
#include <tuple>
#include <type_traits>
#include "proton/proton.hpp"

namespace proton {

template <_res_or_bundle... Resources>
struct res : public std::tuple<Resources&...> {
    static_assert((std::is_same_v<std::remove_const_t<Resources>, Resources> && ...));

    using tuple_type = std::tuple<Resources&...>;
    using std::tuple<Resources&...>::tuple;
    constexpr res(const tuple_type& tup) noexcept // it holds references
        : tuple_type(tup) {}
};

} // namespace proton

template <proton::_res_or_bundle... Resources>
struct std::tuple_size<proton::res<Resources...>>
    : std::integral_constant<size_t, sizeof...(Resources)> {};

template <size_t Index, proton::_res_or_bundle... Resources>
struct std::tuple_element<Index, proton::res<Resources...>> {
    using type = std::tuple_element_t<Index, std::tuple<Resources&...>>;
};
