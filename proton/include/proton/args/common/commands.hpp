#pragma once
#include <cstdint>
#include <utility>
#include "proton/proton.hpp"
#include "proton/world.hpp"

namespace proton {

class commands {
public:
    template <_world World>
    explicit commands(World& world) {}

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
};

} // namespace proton
