#include <cstddef>
#include <mutex>
#include <sstream>
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

void task1(commands);
void task2(commands);
void task3(commands);

using enum stage;
constexpr auto world = world_desc | add_system<update, &task1> |
                       add_system<update, &task2> |
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

void task1(commands cmds) {
    const auto tid = std::this_thread::get_id();
    std::ostringstream oss;
    oss << "tid: " << tid
        << ", task1, cb: " << static_cast<void*>(cmds.get_command_buffer())
        << '\n';
    std::lock_guard guard{ mutex };
    std::cout << oss.str();
}
void task2(commands cmds) {
    const auto tid = std::this_thread::get_id();
    std::ostringstream oss;
    oss << "tid: " << tid << ", task2, cb: " << static_cast<void*>(&cmds)
        << '\n';
    std::lock_guard guard{ mutex };
    std::cout << oss.str();
}
void task3(commands cmds) {
    const auto tid = std::this_thread::get_id();
    std::ostringstream oss;
    oss << "tid: " << tid << ", task3, cb: " << static_cast<void*>(&cmds)
        << '\n';
    std::lock_guard guard{ mutex };
    std::cout << oss.str();
}
