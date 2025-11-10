#include "example_application.hpp"
#include <chrono>
#include <cstdint>
#include <string>
#include <neutron/print.hpp>
#include <neutron/template_list.hpp>
#include <proton/args/common/commands.hpp>
#include <proton/args/common/query.hpp>
#include <proton/args/common/single.hpp>
#include <proton/args/system/local.hpp>
#include <proton/args/system/res.hpp>
#include <proton/observer.hpp>
#include <proton/proton.hpp>
#include <proton/registry.hpp>
#include <proton/stage.hpp>
#include <proton/system.hpp>
#include <proton/world.hpp>

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
        left  = 0,
        up    = 1,
        right = 2,
        down  = 3
    } value;
};
using transform = bundle<position, direction>;
struct sprite {
    using component_tag = void;
    uint32_t handle;
};
struct player {
    using component_tag = void;
};

struct sprite_manager {};
template <typename Ty>
struct input : public Ty {
    using resource_tag = void;
};
class keyboard {
public:
    [[nodiscard]] bool pressing(int key) const { return false; }
};
class timer {
    std::chrono::time_point<std::chrono::system_clock> now_;

public:
    using resource_tag = void;

    [[nodiscard]] auto update() const { return std::chrono::system_clock::now(); }
};

void create_entities(commands commands);
void echo_entities(query<with<const name&, health>>);
void movement(res<const input<keyboard>&, const timer&>, single<with<transform&, player>>);
void update_position(query<with<position&, direction>>);
void render_objs(
    query<with<const transform&, sprite>, without<>>, /*local<spirit_manager>,*/ res<const timer&>);
void modify_game_state(res<game_state>, query<with<health, player>, changed<health>>);

using enum stage;

// clang-format off
constexpr auto world = world_desc
    | add_system<startup, create_entities>
    | add_system<pre_update, movement>
    | add_system<update, update_position>
    | add_system<render, render_objs>
    | add_system<post_update, modify_game_state>
    | add_system<post_update, echo_entities, after<modify_game_state>>
    // | add_observer<update, observer>
    ;
// clang-format on

using reg   = _registry<::world>;
using syss  = _registry<::world>::systems::all_systems;
using qrys  = _registry<::world>::querys;
using comps = _registry<::world>::components;
// using comps  = reg::components;
using psdsys = reg::systems;
using slist  = _registry<::world>::system_list;
using res_t  = reg::resources;
using locals = reg::locals;
using obses  = reg::observers;

int main(int argc, char* argv[]) {
    myapp::create() | run_worlds<::world>();
    return 0;
}

void create_entities(commands commands) {}
void update_time(res<timer&> res) {}
void echo_entities(query<with<const name&, health>> qry) {}
void process_keyboard_input(res<input<keyboard>&> res) {
    // get input from device
}
void movement(
    res<const input<keyboard>&, const timer&> res, single<with<transform&, player>> single) {
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
void render_objs(
    query<with<const transform&, sprite>, without<>> qry, /*local<sprite_manager> local,*/
    res<struct timer> res) {}
void modify_game_state(res<game_state>, query<with<health, player>, changed<health>>) {}
