#pragma once
#include "proton/proton.hpp"

#include <cstddef>
#include <ranges>
#include <type_traits>
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
    using _with_t = neutron::type_list_recurse_expose_t<
        bundle, with<Args...>, neutron::same_cvref>;

    template <typename>
    struct _impl;
    template <component... Components>
    struct _impl<with<Components...>> {
        template <typename Archetype>
        static constexpr bool init(const Archetype& archetype) {
            return archetype.template has<std::remove_cvref_t<Components>...>();
        }
    };

    constexpr bool init(auto& archetypes) {
        return _impl<_with_t>::init(archetypes);
    }
    constexpr bool fetch(auto& out, const auto& archetype) { return true; }
};

template <component_like... Args>
struct without {
    using _without_t = neutron::type_list_recurse_expose_t<
        bundle, without<Args...>, neutron::same_cvref>;

    template <typename>
    struct _impl;
    template <component... Components>
    struct _impl<without<Components...>> {
        template <typename Archetype>
        static constexpr void init(const Archetype& archetype) {
            return (
                !archetype.template has<std::remove_cvref_t<Components>>() &&
                ...);
        }
    };

    constexpr bool init(const auto& archetypes) {
        return _impl<_without_t>::init(archetypes);
    }
    constexpr bool fetch(auto& out, const auto& archetype) { return true; }
};

template <component_like... Args>
struct withany {
    using _withany_t = neutron::type_list_recurse_expose_t<
        bundle, withany<Args...>, neutron::same_cvref>;

    template <typename>
    struct _impl;
    template <component... Components>
    struct _impl<withany<Components...>> {
        template <typename Archetype>
        static constexpr bool init(const Archetype& archetype) {
            return (
                archetype.template has<std::remove_cvref_t<Components>>() ||
                ...);
        }
    };

    constexpr bool init(auto& archetypes) {
        return _impl<_withany_t>::init(archetypes);
    }
    constexpr bool fetch(auto& out, const auto& archetype) { return true; }
};

template <component_like... Args>
struct changed {
    using conflict_list = without<Args...>;
    template <typename Archetype>
    constexpr bool init(const Archetype& archetype) {
        return true;
    }
    constexpr bool fetch(auto& out, const auto& archetype) { return true; }
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
struct _is_withany : neutron::is_specific_type_list<withany, Ty> {};
template <typename Ty>
constexpr auto _is_withany_v = _is_withany<Ty>::value;

static_assert(query_filter<with<>>);
static_assert(query_filter<without<>>);
static_assert(query_filter<withany<>>);
static_assert(query_filter<changed<>>);

template <query_filter... Filters>
class query<Filters...> {
public:
// The implementation of `withany` might be somewhat complex, so it will be put
// on hold for now.
// NOLINTNEXTLINE
#if false
    template <typename Filter>
    using _is_with_or_withany =
        std::bool_constant<_is_with_v<Filter> || _is_withany_v<Filter>>;
#else
    template <typename Filter>
    using _is_with_or_withany = std::bool_constant<_is_with_v<Filter>>;
#endif

    using component_list = neutron::type_list_recurse_expose_t<
        bundle,
        neutron::type_list_expose_t<
            with, neutron::type_list_filt_t<_is_with_or_withany, query>>,
        neutron::same_cvref>;

    using view_t  = neutron::type_list_rebind_t<view, component_list>;
    using eview_t = neutron::type_list_rebind_t<eview, component_list>;

    template <typename Ty>
    using _vector_t = std::vector<Ty>;

    template <world World>
    explicit query(World& world) {
        auto& archetypes = world_accessor::archetypes(world);
        for (auto& [hash, archetype] : archetypes) {
            if ((Filters{}.init(archetype) && ...)) {
                eviews_.emplace_back(archetype);
            }
        }
    }

    auto get() noexcept {
        constexpr auto tovw = [](const eview_t& ev) { return view_t{ ev }; };
        return eviews_ | std::views::transform(tovw) | std::views::join;
    }

    auto get_with_entity() noexcept { return eviews_ | std::views::join; }

    NODISCARD size_t size() const noexcept { return eviews_.size(); }

private:
    std::vector<eview_t> eviews_;
};

namespace internal {
template <typename Ty>
using is_query = neutron::is_specific_type_list<query, Ty>;
}

} // namespace proton
