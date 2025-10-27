#pragma once
#include <concepts>
#include <type_traits>
#include "proton/world.hpp"

namespace proton {

template <typename Ty>
class cond {
public:
    template <_std_simple_allocator Alloc>
    explicit cond(basic_world<Alloc>& world) noexcept(
        std::is_nothrow_invocable_r_v<bool, Ty, basic_world<Alloc>&>)
    requires requires {
        { Ty{}(world) } -> std::same_as<bool>;
    }
        : value_(Ty{}(world)) {}

    explicit operator bool() const noexcept { return value_; }

private:
    bool value_;
};

} // namespace proton
