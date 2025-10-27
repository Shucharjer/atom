#pragma once
#include "neutron/type.hpp"
#include "neutron/value_list.hpp"

namespace proton {

template <typename>
struct const_list;
template <template <typename...> typename Template, typename... Args>
struct const_list<Template<Args...>> {
    using type =
        neutron::value_list<(neutron::concepts::value<Args> || neutron::concepts::cref<Args>)...>;
};
template <typename TypeList>
using const_list_t = typename const_list<TypeList>::type;

} // namespace proton
