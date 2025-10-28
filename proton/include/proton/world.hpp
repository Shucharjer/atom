#pragma once
#include <cstddef>
#include <memory>
#include <type_traits>
#include <vector>
#include <neutron/shift_map.hpp>
#include <neutron/type_hash.hpp>
#include "args/common/command_buffer.hpp"
#include "proton.hpp"
#include "proton/archetype.hpp"
#include "proton/proton.hpp"

namespace proton {

template <
    typename Components, typename Systems, typename Observers, typename Locals, typename Res,
    _std_simple_allocator Alloc = std::allocator<std::byte>>
class basic_world;

template <
    template <typename...> typename CompList, component... Comps, typename Systems,
    typename Observers, typename Locals, typename Res, _std_simple_allocator Alloc>
class basic_world<CompList<Comps...>, Systems, Observers, Locals, Res, Alloc> {
    friend struct world_accessor;

    template <typename Ty>
    using allocator_t = typename std::allocator_traits<Alloc>::template rebind_alloc<Ty>;

public:
    using components = CompList<Comps...>;

    basic_world(const Alloc& alloc = Alloc{}) : archetypes_(alloc), entities_(alloc) {}

private:
    std::vector<archetype, allocator_t<archetype>> archetypes_;
    neutron::shift_map<uint64_t, uint32_t, Alloc> entities_;
    basic_command_buffer<Alloc> command_buffers_;
    Locals locals_;
    Res resources_;

    // constexpr static auto components_hash = neutron::make_hash_array<components>();
};

template <typename>
struct _is_basic_world : std::false_type {};
template <typename... Args>
struct _is_basic_world<basic_world<Args...>> : std::true_type {};
template <typename Ty>
constexpr auto _is_basic_world_v = _is_basic_world<Ty>::value;

template <typename Ty>
concept _world = _is_basic_world_v<Ty>;

struct world_accessor {
    template <typename World>
    requires _is_basic_world<World>::value
    static auto& archetypes(World& world) noexcept {
        return world.archetypes_;
    }
    template <typename World>
    requires _is_basic_world<World>::value
    static auto& entities(World& world) noexcept {
        return world.entities_;
    }
};

} // namespace proton
