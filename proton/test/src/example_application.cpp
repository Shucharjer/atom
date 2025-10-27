#include "example_application.hpp"
#include <string>
#include <neutron/type_list.hpp>
#include <neutron/value_list.hpp>
#include <proton/args/common/commands.hpp>
#include <proton/args/common/query.hpp>
#include <proton/args/system/local.hpp>
#include <proton/args/system/res.hpp>
#include <proton/system.hpp>
#include <proton/world.hpp>
#include "proton/observer.hpp"

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

void create_entities(commands commands);
void echo_entities(query<const name&, health>);
void modify_game_state(res<game_state>, query<health&>);
void observer();

using enum stage;

// clang-format off
constexpr auto world = world_desc
    | add_system<startup, create_entities>
    | add_system<update, echo_entities>
    | add_system<post_update, modify_game_state, after<echo_entities>>
    | add_observer<update, observer>;
// clang-format on

using t              = decltype(::world);
using systems        = extract_systems_t<::world>;
using parsed_systems = parse_system_list<systems>;
using updates        = parsed_systems::_update_systems;
using observers      = extract_observers_t<::world>;

int main(int argc, char* argv[]) {
    myapp::create() | run_worlds<::world>();
    return 0;
}

void create_entities(commands commands) {}
void echo_entities(query<const name&, health> qry) {}
void modify_game_state(res<game_state> res, query<health&> qry) {}
void observer() {}
