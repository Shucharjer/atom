#include <cstddef>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <exec/static_thread_pool.hpp>
#include <neutron/execution.hpp>
#include <neutron/print.hpp>
#include <proton/args/common/commands.hpp>
#include <proton/command_buffer.hpp>
#include <proton/proton.hpp>
#include <proton/registry.hpp>
#include <proton/stage.hpp>
#include <proton/system.hpp>
#include <proton/world.hpp>

using namespace neutron;
using namespace execution;
using namespace proton;

using commands = basic_commands<>;

template <>
constexpr bool as_component<std::string> = true;

void task1(commands cmds);
void task2(commands cmds, query<with<std::string&>> query);
void task3(commands cmds, query<with<const std::string&>> query);

using enum stage;
constexpr auto world = world_desc | add_system<update, &task1> |
                       add_system<update, &task2, before<task1>> |
                       add_system<update, &task3, after<task2>>;

int main() {
    exec::static_thread_pool pool;
    std::vector<command_buffer<>> cmdbufs(pool.available_parallelism());
    scheduler auto sch = pool.get_scheduler();

    auto worlds = make_worlds<::world>();

    for (auto frame = 0; frame < 4; ++frame) {
        println("frame {}", frame);
        call<update>(sch, cmdbufs, worlds);
    }

    return 0;
}

std::mutex mutex;

void task1(commands cmds) { cmds.spawn<std::string>(std::string{ "t1" }); }
void task2(commands cmds, query<with<std::string&>> query) {
    for (auto [string] : query.get()) {
        string = "t2";
    }
}
void task3(commands cmds, query<with<const std::string&>> query) {
    std::ostringstream oss;
    for (auto [string] : query.get()) {
        oss << "string: " << string << '\n';
    }
    std::lock_guard guard{ mutex };
    std::cout << oss.str();
}
