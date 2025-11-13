#pragma once
#include <cstdint>
#include <numeric>
#include <unordered_map>
#include <vector>
#include <neutron/template_list.hpp>
#include "proton/archetype.hpp"
#include "proton/args/qualifier.hpp"
#include "proton/proton.hpp"
#include "proton/world.hpp"

namespace proton {

template <typename...>
class query {
    // only for type deducing
    consteval query() noexcept = default;
};

class _query_iterator {
public:
    _query_iterator();
    _query_iterator& operator++();
    _query_iterator operator++(int);
    auto operator*();
    auto operator->();

private:
};

template <_comp_or_bundle... Comps>
struct changed {};

template <typename>
struct query_filter;
template <typename... Args>
struct query_filter<with<Args...>> {
    static void init(auto& result, const auto& archetypes) noexcept {}
    static void filt(auto& result, const auto& archetypes) {}
};

template <typename>
struct query_filter;
template <typename... Args>
struct query_filter<without<Args...>> {
    static void init(auto& result, const auto& archetypes) noexcept {}
    static void filt(auto& result, const auto& archetypes) {}
};

template <typename>
struct query_filter;
template <typename... Args>
struct query_filter<withany<Args...>> {
    static void init(auto& result, const auto& archetypes) noexcept {}
    static void filt(auto& result, const auto& archetypes) {}
};

template <typename>
struct query_filter;
template <typename... Args>
struct query_filter<changed<Args...>> {
    static void init(auto& result, const auto& archetypes) noexcept {}
    static void filt(auto& result, const auto& archetypes) {}
};

template <typename Ty>
concept _query_filter =
    requires(std::vector<id_t>& result, 
        // prefer const auto&
        const std::vector<archetype<>>& archetypes) {
        Ty::init(result, archetypes);
        Ty::filt(result, archetypes);
    };

template <typename... Filters>
requires(_with_obj_assert<Filters...>() && (_query_filter<query_filter<Filters>> && ...))
class query<Filters...> {
public:
    template <_world World>
    explicit query(World& world) {
        auto& archetypes = world_accessor::archetypes(world);
        archetypes_.resize(archetypes.size());
        std::iota(archetypes_.begin(), archetypes_.end(), 0);
        (query_filter<Filters>::init(archetypes_, archetypes), ...);
    }

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
