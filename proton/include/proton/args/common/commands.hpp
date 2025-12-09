#pragma once
#include <concepts>
#include <utility>
#include "proton/command_buffer.hpp"
#include "proton/proton.hpp"

namespace proton {

class commands {
public:
    template <typename CommandBuffer>
    requires std::convertible_to<
        CommandBuffer&, _command_buffer::_command_buffer_base&>
    explicit commands(CommandBuffer& command_buffer) noexcept
        : command_buffer_(&command_buffer) {}

    future_entity_t spawn() { return command_buffer_->spawn(); }

    template <component... Components>
    requires(std::same_as<Components, std::remove_cvref_t<Components>> && ...)
    future_entity_t spawn() {
        return command_buffer_->spawn<Components...>();
    }

    template <component... Components>
    future_entity_t spawn(Components&&... components) {
        return command_buffer_->spawn(std::forward<Components>(components)...);
    }

    template <component... Components>
    void add_components(future_entity_t entity) {
        return command_buffer_->add_components<Components...>(entity);
    }

    template <component... Components>
    void add_components(entity_t entity) {
        return command_buffer_->add_components<Components...>(entity);
    }

    template <typename Entity, component... Components>
    void add_components(Entity entity, Components&&... components) {
        return command_buffer_->add_components(
            entity, std::forward<Components>(components)...);
    }

    template <component... Components>
    void remove_components(future_entity_t entity) {
        return command_buffer_->remove_components<Components...>(entity);
    }

    template <component... Components>
    void remove_components(entity_t entity) {
        return command_buffer_->remove_components<Components...>(entity);
    }

    void kill(future_entity_t entity);

    void kill(entity_t entity);

private:
    _command_buffer::_command_buffer_base* command_buffer_;
};

} // namespace proton
