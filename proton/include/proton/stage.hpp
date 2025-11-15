#pragma once
#include <cstdint>

namespace proton {

enum class stage : uint8_t {
    pre_startup,
    startup,
    post_startup,
    first,
    pre_update,
    update,
    post_update,
    render,
    last,
    shutdown
};

template <stage Stage, typename...>
struct staged_type_list {};

template <stage Stage, typename TypeList>
struct staged_type_list_from_value;
template <
    stage Stage, template <typename...> typename Template, typename... Tys>
struct staged_type_list_from_value<Stage, Template<Tys...>> {
    using type = staged_type_list<Stage, Tys...>;
};
template <stage Stage, typename TypeList>
using staged_type_list_from_value_t =
    typename staged_type_list_from_value<Stage, TypeList>::type;

template <stage Stage, auto...>
struct staged_value_list {};

template <stage Stage, typename ValueList>
struct staged_value_list_from_value;
template <stage Stage, template <auto...> typename Template, auto... Values>
struct staged_value_list_from_value<Stage, Template<Values...>> {
    using type = staged_value_list<Stage, Values...>;
};
template <stage Stage, typename ValueList>
using staged_value_list_from_value_t =
    typename staged_value_list_from_value<Stage, ValueList>::type;

} // namespace proton
