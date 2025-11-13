#pragma once
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include "neutron/shift_map.hpp"
#include "proton.hpp"
#include "proton/archetype.hpp"
#include "proton/proton.hpp"

namespace proton {

template <_std_simple_allocator Alloc = std::allocator<std::byte>>
class command_buffer {
    template <typename Ty>
    using _rebind_alloc_t = neutron::rebind_alloc_t<Alloc, Ty>;

public:
    constexpr void new_frame() { inframe_index_.store(0); }

    constexpr future_entity_t spawn() {
        auto curr = inframe_index_.fetch_add(1);
        // put to buf
        return future_entity_t{ curr };
    }

    template <typename... Components>
    constexpr void add_components(entity_t entity) {
        //
    }

    template <typename... Components>
    constexpr void add_components(future_entity_t entity) {
        //
    }

    template <typename... Components>
    constexpr void add_components(entity_t, Components&&... components) {
        //
    }

    template <typename... Components>
    constexpr void add_components(future_entity_t, Components&&... components) {
        //
    }

    template <typename... Components>
    constexpr void remove_components() {
        //
    }

    constexpr void kill(entity_t entity) {
        //
    }

    constexpr void kill(future_entity_t entity) {
        //
    }

    template <
        typename ArchetypeAlloc, typename ArchetypesAlloc, size_t PageSize, size_t Shift,
        typename EntitiesAlloc>
    constexpr void execute(
        std::vector<archetype<ArchetypeAlloc>, ArchetypesAlloc>& archetypes,
        neutron::shift_map<entity_t, index_t, PageSize, Shift, EntitiesAlloc>& entities) {
        //
    }

private:
    enum class command : uint8_t {
        spawn  = 0,
        add    = 1,
        remove = 2,
        kill   = 3
    };
    struct alignas(64) command_block {
        void (*command)();
    };

    std::vector<command_block, _rebind_alloc_t<command_block>> commands_;
    std::vector<std::byte, _rebind_alloc_t<std::byte>> buffer_;

    std::atomic<index_t> inframe_index_;
};

} // namespace proton
