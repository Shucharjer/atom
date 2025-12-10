#pragma once
#include "proton/proton.hpp"

#include <cstdint>
#include <numeric>
#include <unordered_map>
#include <vector>
#include <neutron/template_list.hpp>
#include "proton/archetype.hpp"
#include "proton/world_accessor.hpp"

namespace proton {

class _query_iterator {
public:
    _query_iterator();
    _query_iterator& operator++();
    _query_iterator operator++(int);
    auto operator*();
    auto operator->();

private:
};

template <component_like...>
struct with;
template <component_like...>
struct without;
template <component_like...>
struct withany;
template <component_like...>
struct changed;

template <component_like... Args>
struct with {
    using conflict_list = without<Args...>;
    constexpr void init(auto& out, const auto& archetypes) {}
    constexpr void fetch(auto& out, const auto& archetype) {}
};

template <component_like... Args>
struct without {
    using confilct_list = with<Args...>;
    constexpr void init(auto& out, const auto& archetypes) {}
    constexpr void fetch(auto& out, const auto& archetype) {}
};

template <component_like... Args>
struct withany {
    using conflict_list = without<Args...>;
    constexpr void init(auto& out, const auto& archetypes) {}
    constexpr void fetch(auto& out, const auto& archetype) {}
};

template <component_like... Args>
struct changed {
    using conflict_list = without<Args...>;
    constexpr void init(auto& out, const auto& archetypes) {}
    constexpr void fetch(auto& out, const auto& archetype) {}
};

template <typename Ty>
struct _is_with_like {
    constexpr static auto value =
        neutron::is_specific_type_list_v<with, Ty> ||
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

template <query_filter... Filters>
class query<Filters...> {
public:
    template <world World>
    explicit query(World& world) {
        auto& archetypes = world_accessor::archetypes(world);
        archetypes_.resize(archetypes.size());
        std::iota(archetypes_.begin(), archetypes_.end(), 0);
        (Filters::init(archetypes_, archetypes), ...);
    }

    auto get();

    template <component Component>
    auto get();

    template <component... Components>
    requires(sizeof...(Components) > 1)
    auto get();

    auto get_with_entity();

private:
    std::vector<id_t> archetypes_;
};

namespace internal {
template <typename Ty>
using is_query = neutron::is_specific_type_list<query, Ty>;
}

} // namespace proton
