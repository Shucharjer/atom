#pragma once
#include <atomic>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <type_traits>
#include <vector>
#include "neutron/shift_map.hpp"
#include "proton.hpp"
#include "proton/archetype.hpp"
#include "proton/proton.hpp"

namespace proton {

/*! @cond TURN_OFF_DOXYGEN */
namespace _command_buffer {

class _command_buffer_base;

struct command {
    enum class type : uint8_t {
        spawn,
        attach,
        add,
        detach,
        remove,
        kill
    };
    uint8_t count;
    uint16_t size;
};

struct _command_buffer_context {
    command* commands{};
    size_t command_size{};
    size_t command_capcity{};
    std::byte* buffer{};
    size_t buffer_size{};
    size_t buffer_capacity{};
};

class alignas(std::hardware_destructive_interference_size)
    _command_buffer_base {

    using grow_fn = _command_buffer_context (*)(_command_buffer_base*);

public:
    template <typename CommandBuffer>
    requires std::derived_from<
        std::remove_cvref_t<CommandBuffer>, _command_buffer_base>
    constexpr _command_buffer_base(
        [[maybe_unused]] CommandBuffer* command_buffer) noexcept
        : grow_([](_command_buffer_base* self) -> _command_buffer_context {
              return static_cast<std::remove_cvref_t<CommandBuffer>*>(self)
                  ->_grow();
          }) {}

    _command_buffer_base(const _command_buffer_base&)            = delete;
    _command_buffer_base& operator=(const _command_buffer_base&) = delete;

    constexpr _command_buffer_base(_command_buffer_base&&) noexcept = default;
    constexpr _command_buffer_base&
        operator=(_command_buffer_base&&) noexcept = default;

    constexpr ~_command_buffer_base() noexcept = default;

    constexpr void reset() noexcept {
        inframe_index_        = 0;
        context_.command_size = 0;
        context_.buffer_size  = 0;
    }

    future_entity_t spawn() noexcept {
        const future_entity_t entity{ inframe_index_++ };
        // emplace command spawn
        return entity;
    }

    template <component... Components>
    future_entity_t spawn() noexcept {
        const future_entity_t entity{ inframe_index_++ };
        // emplace command spawn & command add
        return entity;
    }

    template <component... Components>
    future_entity_t spawn(Components&&... components) noexcept {
        const future_entity_t entity{ inframe_index_++ };
        // emplace command spawn & command add
        return entity;
    }

    template <component... Components>
    void add_components(future_entity_t entity) {
        // emplace command add
    }

    template <component... Components>
    void add_components(entity_t entity) {
        // emplace command attach
    }

    template <component... Components>
    void add_components(future_entity_t entity, Components&&... components) {
        // emplace command add
    }

    template <component... Components>
    void add_components(entity_t entity, Components&&... components) {
        // emplace command attach
    }

    template <component... Components>
    void remove_components(future_entity_t entity) {
        // emplace command remove
    }

    template <component... Components>
    void remove_components(entity_t entity) {
        // emplace command detach
    }

    void kill(future_entity_t entity) {
        // emplace command kill
    }

    void kill(entity_t entity) {
        // emplace command kill
    }

private:
    void _emplace_command(command cmd, void* payload) {
        if (context_.command_size == context_.command_capcity ||
            context_.buffer_size + cmd.size > context_.buffer_capacity)
            [[unlikely]] {
            context_ = grow_(this);
        }

        context_.commands[context_.command_size++] = cmd;
        // copy payload
    }

    index_t inframe_index_{};
    _command_buffer_context context_;
    grow_fn grow_;
};

} // namespace _command_buffer
/*! @endcond */

/**
 * @class command_buffer
 * @brief A buffer stores commands in a single thread.
 *
 * @tparam Alloc
 */
template <_std_simple_allocator Alloc = std::allocator<std::byte>>
class alignas(std::hardware_destructive_interference_size) command_buffer :
    public _command_buffer::_command_buffer_base {
    using _base_type = _command_buffer::_command_buffer_base;

    template <typename Ty>
    using _rebind_alloc_t = neutron::rebind_alloc_t<Alloc, Ty>;

    friend class _command_buffer::_command_buffer_base;

public:
    template <typename Al = Alloc>
    constexpr command_buffer(const Al& alloc = {}) : _base_type(this) {}

private:
    _command_buffer::_command_buffer_context _grow() {
        //
    }
};

} // namespace proton
