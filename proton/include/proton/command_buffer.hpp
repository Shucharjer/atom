#pragma once
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <vector>
#include "neutron/shift_map.hpp"
#include "proton.hpp"
#include "proton/archetype.hpp"
#include "proton/proton.hpp"

namespace proton {

/**
 * @class command_buffer
 * @brief A buffer stores commands in a single thread.
 *
 * @tparam Alloc
 */
template <_std_simple_allocator Alloc = std::allocator<std::byte>>
class alignas(std::hardware_destructive_interference_size) command_buffer {
    template <typename Ty>
    using _rebind_alloc_t = neutron::rebind_alloc_t<Alloc, Ty>;

public:
    inline static thread_local command_buffer* buffer = nullptr;

    constexpr void reset() {
        inframe_index_ = 0;
        commands_.clear();
    }

    constexpr future_entity_t spawn() {
        auto curr = inframe_index_++;
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

    NODISCARD const auto& get() const noexcept { return commands_; }

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

    index_t inframe_index_;
};

} // namespace proton
