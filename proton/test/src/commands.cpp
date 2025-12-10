#include <proton/args/common/commands.hpp>
#include <proton/command_buffer.hpp>
#include <neutron/print.hpp>

using namespace neutron;
using namespace proton;

int main() {
    command_buffer<> command_buffer;
    commands cmds{ command_buffer };

    auto entity = cmds.spawn();
    println("entity {}", entity);

    command_buffer.reset();

    return 0;
}
