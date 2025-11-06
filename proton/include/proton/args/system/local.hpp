#pragma once
#include <tuple>
#include <type_traits>

namespace proton {

template <typename... Args>
struct local : public std::tuple<Args&...> {
    static_assert((std::is_same_v<std::remove_const_t<Args>, Args> && ...));

    using tuple_type = std::tuple<Args&...>;
    using std::tuple<Args&...>::tuple;
    constexpr local(const tuple_type& tup) noexcept // it holds references
        : tuple_type(tup) {}
};

template <auto System, typename... Args>
struct _local_desc {};

} // namespace proton

template <typename... Args>
struct std::tuple_size<proton::local<Args...>> : std::integral_constant<size_t, sizeof...(Args)> {};

template <size_t Index, typename... Args>
struct std::tuple_element<Index, proton::local<Args...>> {
    using type = std::tuple_element_t<Index, std::tuple<Args&...>>;
};
