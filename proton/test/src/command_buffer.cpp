#include <chrono>
#include <string>
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
template <>
constexpr bool as_component<int> = true;

// constexpr auto loop = 2;
constexpr auto loop = 2'500'000;

void task(commands cmds) {
    using namespace std::chrono;
    auto beg = high_resolution_clock::now();
    for (auto i = 0; i < loop; ++i) {
        cmds.spawn<int>();
    }
    auto end     = high_resolution_clock::now();
    auto elapsed = duration_cast<microseconds>(end - beg);
    println("elapsed: {} seconds", static_cast<float>(elapsed.count()) / 1e6);
}

void task1(commands cmds, [[maybe_unused]] query<with<int>>) { task(cmds); }
void task2(commands cmds) { task(cmds); }
void task3(commands cmds) { task(cmds); }
void task4(commands cmds) { task(cmds); }

using enum stage;
constexpr auto world = world_desc | add_system<update, &task1>;

int main() {
    exec::static_thread_pool pool;
    std::vector<command_buffer<>> cmdbufs(pool.available_parallelism());
    scheduler auto sch = pool.get_scheduler();

    auto world = make_world<::world>();

    const auto tosec = 1e6;

    using namespace std::chrono;
    for (auto frame = 0; frame < 4; ++frame) {
        println("frame {}", frame);
        auto beg = high_resolution_clock::now();
        call<update>(sch, cmdbufs, world);
        auto end     = high_resolution_clock::now();
        auto elapsed = duration_cast<microseconds>(end - beg);
        println(
            "elapsed: {} seconds", static_cast<float>(elapsed.count()) / tosec);

        auto& cmdbuf = cmdbufs[0];
        commands cmds{ cmdbuf };
        query<with<int>> qry{ world };
        cmdbuf.reset();
        for (auto e : qry.entities()) {
            cmds.kill(e); // BUG: duplicated index
        }
        cmdbuf.apply(reinterpret_cast<world_base<>&>(world));
    }

    return 0;
}
