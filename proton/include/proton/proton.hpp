#pragma once
#include <type_traits>
#include <neutron/memory.hpp>

namespace proton {

struct component_tag {};

template <typename Ty>
concept component = requires { typename std::remove_cvref_t<Ty>::component_tag; };

struct resource_tag {};

template <typename Ty>
concept resource = requires { typename std::remove_cvref_t<Ty>::resource_tag; };

using neutron::_std_simple_allocator;

} // namespace proton

/*

sys:
Local, Query, If, Deferred,













*/