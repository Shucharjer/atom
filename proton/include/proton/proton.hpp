#pragma once
#include <type_traits>
#include <neutron/memory.hpp>
#include <neutron/template_list.hpp>

namespace proton {

template <typename...>
struct bundle {};

template <typename>
struct _is_bundle : std::false_type {};
template <typename... Args>
struct _is_bundle<bundle<Args...>> : std::true_type {};

template <typename Ty>
concept _bundle = _is_bundle<Ty>::value;

struct component_tag {};

template <typename Ty>
concept component =
    requires { typename std::remove_cvref_t<Ty>::component_tag; };

template <typename Ty>
struct _is_component {
    constexpr static bool value = component<Ty>;
};

template <typename Ty>
concept _comp_or_bundle =
    component<Ty> ||
    (_bundle<Ty> &&
     neutron::type_list_requires_recurse<_is_component, Ty>::value);

struct resource_tag {};

template <typename Ty>
concept resource = requires { typename std::remove_cvref_t<Ty>::resource_tag; };

template <typename Ty>
struct _is_resource {
    constexpr static bool value = resource<Ty>;
};

template <typename Ty>
concept _res_or_bundle =
    resource<Ty> ||
    (_bundle<Ty> &&
     neutron::type_list_requires_recurse<_is_resource, Ty>::value);

using neutron::_std_simple_allocator;
using neutron::rebind_alloc_t;

template <
    typename Components, typename Systems, typename Observers, typename Locals,
    typename Res, _std_simple_allocator Alloc = std::allocator<std::byte>>
class basic_world;

template <typename>
struct _is_basic_world : std::false_type {};
template <typename... Args>
struct _is_basic_world<basic_world<Args...>> : std::true_type {};
template <typename Ty>
constexpr auto _is_basic_world_v = _is_basic_world<Ty>::value;

template <typename Ty>
concept _world = _is_basic_world_v<Ty>;

using entity_t     = uint64_t;
using generation_t = uint32_t;
using index_t      = uint32_t;

class future_entity_t {
public:
    constexpr explicit future_entity_t(index_t inframe_index)
        : identity_(inframe_index) {}

    NODISCARD index_t get() const noexcept { return identity_; }

private:
    index_t identity_;
};

} // namespace proton
