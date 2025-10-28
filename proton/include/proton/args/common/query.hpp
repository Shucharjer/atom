#pragma once
#include <neutron/type_list.hpp>
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

template <typename Require, component... Components>
requires(!component<Require>)
class query<Require, Components...> {
public:
    template <_world World>
    explicit query(World& world) {}

    template <component Component>
    auto get() {}
};

template <component... Components>
class query<Components...> {
public:
    template <_world World>
    explicit query(World& world) {}

    template <component Component>
    auto get() {}
};

template <component... Components>
requires(std::is_same_v<Components, std::remove_cvref_t<Components>> && ...)
struct without {};

namespace internal {
template <typename Ty>
using is_query = neutron::is_specific_type_list<query, Ty>;
}

} // namespace proton
