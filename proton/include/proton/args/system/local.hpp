#pragma once
#include <tuple>
#include <type_traits>
#include "neutron/template_list.hpp"
#include "proton/proton.hpp"
#include "proton/system.hpp"
#include "proton/world_accessor.hpp"

namespace proton {

template <typename... Args>
class local {
public:
    static_assert((std::is_same_v<std::remove_const_t<Args>, Args> && ...));

    template <auto Sys>
    constexpr local(_sys_tuple<Sys, Args...>& tup) noexcept : tup_(tup) {}

    template <size_t Index>
    constexpr decltype(auto) get() noexcept {
        return std::get<Index>(tup_);
    }

    template <size_t Index>
    constexpr decltype(auto) get() const noexcept {
        return std::get<Index>(tup_);
    }

private:
    std::tuple<Args...>& tup_; // NOLINT
};

template <auto Sys, size_t Index, typename... Args>
struct construct_from_world_t<Sys, local<Args...>, Index> {
    template <typename Ty>
    using _predicate = _is_relevant_sys_tuple<Sys, Ty>;

    template <world World>
    local<Args...> operator()(World& world) const noexcept {
        using sys_tuple = neutron::type_list_first_t<
            neutron::type_list_filt_t<_predicate, typename World::locals>>;
        auto& locals = world_accessor::locals(world);
        return neutron::get_first<sys_tuple>(locals);
    }
};

} // namespace proton

template <typename... Args>
struct std::tuple_size<proton::local<Args...>> :
    std::integral_constant<size_t, sizeof...(Args)> {};

template <size_t Index, typename... Args>
struct std::tuple_element<Index, proton::local<Args...>> {
    using type = std::tuple_element_t<Index, std::tuple<Args&...>>;
};
