#include <string>
#include <proton/proton.hpp>
#include <proton/world_base.hpp>

using namespace proton;

template <>
constexpr inline bool as_component<std::string> = true;

int main() {
    world_base<> world;
    auto e = world.spawn();
    world.add_components<std::string>(e);
    world.remove_components<std::string>(e);
    world.kill(e);

    return 0;
}
