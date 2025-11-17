#pragma once
#include <cstddef>
#include <memory_resource>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>
// #include <neutron/thread_pool.hpp>
#include <exec/__detail/__numa.hpp>
#include <exec/static_thread_pool.hpp>
#include <proton/execution.hpp>
#include <proton/registry.hpp>
#include "proton/command_buffer.hpp"
#include "proton/stage.hpp"
#include "proton/world.hpp"

class myapp {
public:
    using config_type = std::tuple<>;
    static myapp create() { return {}; }
    template <auto World> // single world
    void run() {
        using namespace proton;
        using namespace execution;
        using enum stage;

        // create allocator
        constexpr auto bufsize = 1024 * 1024 * 1;
        std::vector<std::byte> buf(bufsize);
        auto upstream = std::pmr::monotonic_buffer_resource{};
        auto pool     = std::pmr::synchronized_pool_resource{ &upstream };
        auto alloc    = std::pmr::polymorphic_allocator<>{ &pool };

        // create execution environment

        const auto concurrency = std::thread::hardware_concurrency();
        auto thread_pool       = exec::static_thread_pool{ concurrency };
        scheduler auto sch     = thread_pool.get_scheduler();
        auto sender            = sch.schedule();
        using command_buffer   = command_buffer<decltype(alloc)>;
        auto command_buffers   = std::vector<
              command_buffer, rebind_alloc_t<decltype(alloc), command_buffer>>{
            32
        };

        auto worlds = make_worlds<World>(alloc);
        call_startup(worlds, sch);
        call_update(worlds, sch);
        call<shutdown>(worlds, sch);
    }
};
