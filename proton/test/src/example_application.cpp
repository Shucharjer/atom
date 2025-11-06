#include "example_application.hpp"
#include <cstdint>
#include <string>
#include <neutron/type_list.hpp>
#include <neutron/value_list.hpp>
#include <proton/args/common/commands.hpp>
#include <proton/args/common/query.hpp>
#include <proton/args/system/local.hpp>
#include <proton/args/system/res.hpp>
#include <proton/observer.hpp>
#include <proton/registry.hpp>
#include <proton/system.hpp>
#include <proton/world.hpp>
#include "proton/proton.hpp"

using namespace proton;

struct name {
    using component_tag = void;
    std::string value;
};
struct health {
    using component_tag = void;
    float value;
};
struct game_state {
    using resource_tag = void;
    enum _state : uint8_t {
        menu,
        started,
        lose
    };

    _state value;
};
struct position {
    using component_tag = void;
    int x;
    int y;
};
struct direction {
    using component_tag = void;
    enum : uint8_t {
        left,
        up,
        right,
        down
    } value;
};
using transform = bundle<position, direction>;
struct mesh {
    using component_tag = void;
    uint32_t handle;
};
struct player {
    using component_tag = void;
};

struct mesh_manager {};

void create_entities(commands commands);
void echo_entities(query<const name&, health>);
void movement(query<transform>);
void render_objs(query<transform, mesh>, local<mesh_manager>);
void modify_game_state(res<game_state>, query<health, player>);
void observer();

using enum stage;

// clang-format off
constexpr auto world = world_desc
    | add_system<startup, create_entities>
    | add_system<update, echo_entities>
    | add_system<update, movement, after<echo_entities>>
    | add_system<render, render_objs>
    | add_system<post_update, modify_game_state, after<echo_entities>>
    | add_observer<update, observer>;
// clang-format on

using reg    = registry<::world>;
using comps  = reg::components;
using psdsys = reg::systems;
using slist  = _registry<::world>::system_list;
using res_t  = reg::resources;
using locals = reg::locals;
using obses  = reg::observers;

int main(int argc, char* argv[]) {
    myapp::create() | run_worlds<::world>();
    auto world = make_worlds<::world>();
    return 0;
}

void create_entities(commands commands) {}
void echo_entities(query<const name&, health> qry) {}
void modify_game_state(res<game_state> res, query<health, player> qry) {}
void observer() {}
