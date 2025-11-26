#pragma once
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <neutron/template_list.hpp>
#include "proton/proton.hpp"
#include "proton/world.hpp"

namespace proton {

template <typename... Resources>
requires(resource_like<std::remove_cvref_t<Resources>> && ...)
struct res : public std::tuple<Resources...> {
    template <world World>
    res(World& world)
        : std::tuple<Resources...>(
              neutron::get_first<std::remove_cvref_t<Resources>>(
                  world_accessor::resources(world))...) {}
};

} // namespace proton

template <typename... Resources>
struct std::tuple_size<proton::res<Resources...>> :
    std::integral_constant<size_t, sizeof...(Resources)> {};

template <size_t Index, proton::resource_like... Resources>
struct std::tuple_element<Index, proton::res<Resources...>> {
    using type = std::tuple_element_t<Index, std::tuple<Resources&...>>;
};
