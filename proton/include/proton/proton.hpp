#pragma once
#include <concepts>
#include <memory>
#include <type_traits>
#include <neutron/memory.hpp>
#include <neutron/pipeline.hpp>
#include <neutron/template_list.hpp>

namespace proton {

template <typename... Content>
struct world_descriptor_t {};

constexpr inline world_descriptor_t<> world_desc;

template <typename>
constexpr bool _is_world_descriptor = false;
template <typename... Content>
constexpr bool _is_world_descriptor<world_descriptor_t<Content...>> = true;

template <typename Descriptor>
concept world_descriptor =
    _is_world_descriptor<std::remove_cvref_t<Descriptor>>;

template <typename Derived>
struct descriptor_adaptor_closure : neutron::adaptor_closure<Derived> {};

template <typename Closure>
concept descriptor_closure =
    neutron::_adaptor_closure<Closure, descriptor_adaptor_closure>;

template <typename First, typename Second>
struct descriptor_closure_compose :
    neutron::_closure_compose<
        descriptor_closure_compose, descriptor_adaptor_closure, First, Second> {
    using _compose_base = neutron::_closure_compose<
        descriptor_closure_compose, descriptor_adaptor_closure, First, Second>;
    using _compose_base::_compose_base;
    using _compose_base::operator();
};

template <typename C1, typename C2>
descriptor_closure_compose(C1&&, C2&&) -> descriptor_closure_compose<
    std::remove_cvref_t<C1>, std::remove_cvref_t<C2>>;

template <world_descriptor Descriptor, descriptor_closure Closure>
constexpr decltype(auto)
    operator|(Descriptor&& descriptor, Closure&& closure) noexcept {
    return std::forward<Closure>(closure)(std::forward<Descriptor>(descriptor));
}

template <descriptor_closure Closure1, descriptor_closure Closure2>
constexpr decltype(auto)
    operator|(Closure1&& closure1, Closure2&& closure2) noexcept {
    return descriptor_closure_compose(
        std::forward<Closure1>(closure1), std::forward<Closure2>(closure2));
}

template <typename...>
struct bundle {};

/*! @cond TURN_OFF_DOXYGEN */
namespace internal {

template <typename>
struct _is_bundle : std::false_type {};
template <typename... Args>
struct _is_bundle<bundle<Args...>> : std::true_type {};

} // namespace internal
/* @endcond */

template <typename Ty>
concept _bundle = internal::_is_bundle<std::remove_cvref_t<Ty>>::value;

struct component_t {};

template <typename Ty>
constexpr bool as_component = false;

template <typename Ty>
concept component = requires {
    typename std::remove_cvref_t<Ty>::component_concept;
    requires std::derived_from<
        typename std::remove_cvref_t<Ty>::component_concept, component_t>;
} || as_component<Ty>;

/*! @cond TURN_OFF_DOXYGEN */
namespace internal {

template <typename Ty>
struct _is_component {
    constexpr static bool value = component<Ty>;
};

} // namespace internal
/* @endcond */

template <typename Ty>
concept component_like =
    component<Ty> ||
    (_bundle<Ty> &&
     neutron::type_list_requires_recurse<
         internal::_is_component, Ty, internal::_is_bundle>::value);

struct tcomp {};
struct tcomp2 {};

template <>
constexpr bool as_component<tcomp> = true;
template <>
constexpr bool as_component<tcomp2> = true;

struct resource_t {};

template <typename Ty>
constexpr bool as_resource = false;

template <typename Ty>
concept resource = requires {
    typename std::remove_cvref_t<Ty>::resource_concept;
    requires std::derived_from<
        typename std::remove_cvref_t<Ty>::resource_concept, resource_t>;
} || as_resource<Ty>;

/*! @cond TURN_OFF_DOXYGEN */
namespace internal {

template <typename Ty>
struct _is_resource {
    constexpr static bool value = resource<Ty>;
};

} // namespace internal
/* @endcond */

template <typename Ty>
concept resource_like =
    resource<Ty> ||
    (_bundle<Ty> &&
     neutron::type_list_requires_recurse<internal::_is_resource, Ty>::value);

using neutron::_std_simple_allocator;
using neutron::rebind_alloc_t;

template <
    typename Registry, _std_simple_allocator Alloc = std::allocator<std::byte>>
class basic_world;

/*! @cond TURN_OFF_DOXYGEN */
namespace internal {

template <typename>
constexpr bool _is_basic_world = false;
template <typename... Args>
constexpr bool _is_basic_world<basic_world<Args...>> = true;
} // namespace internal

/* @endcond */

template <typename Ty>
concept world = internal::_is_basic_world<Ty>;

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
