#include "example_application.hpp"
#include <chrono>
#include <cstdint>
#include <memory_resource>
#include <string>
#include <neutron/print.hpp>
#include <neutron/template_list.hpp>
#include <neutron/type_hash.hpp>
#include <proton/proton.hpp>

#include <proton/args/common/commands.hpp>
#include <proton/args/common/query.hpp>
#include <proton/args/system/local.hpp>
#include <proton/args/system/res.hpp>
#include <proton/observer.hpp>
#include <proton/registry.hpp>
#include <proton/run.hpp>
#include <proton/stage.hpp>
#include <proton/system.hpp>
#include <proton/world.hpp>

using namespace proton;
using commands = basic_commands<std::pmr::polymorphic_allocator<>>;

struct name {
    using component_concept = component_t;
    std::string value;
};
struct health {
    using component_concept = component_t;
    float value;
};
struct game_state {
    using resource_concept = resource_t;
    enum _state : uint8_t {
        menu,
        started,
        lose
    };

    _state value;
};
struct position {
    using component_concept = component_t;
    int x;
    int y;
};
struct direction {
    using component_concept = component_t;
    enum : uint8_t {
        left  = 0,
        up    = 1,
        right = 2,
        down  = 3
    } value;
};
using transform = bundle<position, direction>;
struct sprite {
    using component_concept = component_t;
    uint32_t handle;
};
struct player {
    using component_concept = component_t;
};

struct sprite_manager {};
template <typename Ty>
struct input : public Ty {
    using resource_concept = resource_t;
};
class keyboard {
public:
    [[nodiscard]] bool pressing(int key) const { return false; }
};
class timer {
    std::chrono::time_point<std::chrono::system_clock> now_;

public:
    using resource_concept = resource_t;

    [[nodiscard]] auto update() const {
        return std::chrono::system_clock::now();
    }
};

void create_entities(commands commands);
void echo_entities(query<with<const name&, health>>);
void movement(
    res<const input<keyboard>&, const timer&>, query<with<transform&, player>>);
void update_position(query<with<position&, direction>>);
void echo_position(query<with<position>> qry);
void render_objs(
    query<with<const transform&, sprite>>, local<sprite_manager>,
    res<const timer&>);
void modify_game_state(
    res<game_state>, query<with<health, player>, changed<health>>);

using enum stage;

// clang-format off
constexpr auto world = world_desc
    | add_system<startup, create_entities>
    | add_system<pre_update, movement>
    | add_system<update, update_position>
    | add_system<post_update, echo_position>
    | add_system<render, render_objs>
    | add_system<post_update, modify_game_state>
    | add_system<post_update, echo_entities, after<modify_game_state>>
    // | add_observer<update, observer>
    ;
// clang-format on

int main(int argc, char* argv[]) {
    myapp::create() | run_worlds<::world>();
    return 0;
}

using reg   = registry<::world>;
using _reg  = _registry<::world>;
using syss  = reg::systems::all_systems;
using qrys  = _reg::querys;
using comps = reg::components;
// using comps  = reg::components;
using psdsys = reg::systems;
using slist  = reg::system_list;
using res_t  = reg::resources;
using locals = reg::locals;
using obses  = reg::observers;

void create_entities(commands commands) {}
void update_time(res<timer&> res) {}
void echo_entities(query<with<const name&, health>> qry) {}
void process_keyboard_input(res<input<keyboard>&> res) {
    // get input from device
}
void movement(
    res<const input<keyboard>&, const timer&> res,
    query<with<transform&, player>> single) {
    auto& [keyboard_input, time] = res;
    auto can_move                = [] { return false; };
    //
}
void update_position(query<with<position&, direction>> qry) {
    // std::array update_x = { -1, 0, 1, 0 };
    // std::array update_y = { 0, 1, 0, -1 };
    // for (auto [pos, dir] : qry.get()) {
    //     pos.x += update_x[dir];
    //     pos.y += update_y[dir];
    // }
}
void echo_position(query<with<position>> qry) {
    // for (auto pos : qry.get<position>()) {
    //     neutron::println("{}", pos);
    // }
}
void render_objs(
    query<with<const transform&, sprite>> qry, local<sprite_manager> local,
    res<const timer&> res) {
    auto& [sprite_manager] = local;
}
void modify_game_state(
    res<game_state>, query<with<health, player>, changed<health>>) {}
