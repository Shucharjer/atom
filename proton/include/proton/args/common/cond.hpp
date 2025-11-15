#pragma once
#include <concepts>
#include <type_traits>
#include "proton/world.hpp"

namespace proton {

template <typename Ty>
class cond {
public:
    template <_world World>
    explicit cond(World& world) noexcept(
        std::is_nothrow_invocable_r_v<bool, Ty, World&>)
    requires requires {
        { Ty{}(world) } -> std::same_as<bool>;
    }
        : value_(Ty{}(world)) {}

    explicit operator bool() const noexcept { return value_; }

private:
    bool value_;
};

} // namespace proton
