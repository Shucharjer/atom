#pragma once
#include <neutron/template_list.hpp>
#include "proton/args/match.hpp"
#include "proton/args/qualifier.hpp"
#include "proton/proton.hpp"
#include "proton/world.hpp"

namespace proton {

template <typename...>
class query;

class _query_iterator {
public:
    _query_iterator();
    _query_iterator& operator++();
    _query_iterator operator++(int);

private:
};

template <typename... Args>
requires(_with_obj_assert<Args...>())
class query<Args...> {
public:
    template <_world World>
    explicit query(World& world) {}

    auto get();

    auto get_with_entity();
};

template <_comp_or_bundle... Comps>
struct changed {};

namespace internal {
template <typename Ty>
using is_query = neutron::is_specific_type_list<query, Ty>;
}

} // namespace proton
