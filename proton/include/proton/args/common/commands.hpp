#pragma once
#include <concepts>
#include <cstdint>
#include <utility>
#include "proton/command_buffer.hpp"
#include "proton/proton.hpp"

namespace proton {

class commands {
public:
    template <typename CommandBuffer>
    requires std::convertible_to<CommandBuffer&, class command_buffer_base&>
    explicit commands(CommandBuffer& command_buffer) noexcept
        : command_buffer_(&command_buffer) {}

    std::pair<entity_t, bool> spawn();

    template <_comp_or_bundle... Components>
    std::pair<entity_t, bool> spawn();

    template <_comp_or_bundle... Components>
    std::pair<entity_t, bool> spawn(Components&&... components);

    template <_comp_or_bundle... Components>
    void append(entity_t entity);

    template <_comp_or_bundle... Components>
    void append(entity_t entity, Components&&... components);

    template <_comp_or_bundle... Components>
    void remove(entity_t entity);

    void kill(uint64_t entity);

private:
    command_buffer_base* command_buffer_;
};

} // namespace proton
