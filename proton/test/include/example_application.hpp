#pragma once
#include <cstddef>
#include <memory_resource>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>
// #include <neutron/thread_pool.hpp>
#include <neutron/parallel.hpp>
#include <proton/registry.hpp>
#include "proton/command_buffer.hpp"
#include "proton/stage.hpp"
#include "proton/world.hpp"

template <typename Alloc>
struct context_maker {
    template <typename Al = Alloc>
    constexpr auto operator()(const Al& alloc = {})
        -> proton::command_buffer<Alloc> {
        return proton::command_buffer<Alloc>{ alloc };
    }
};

class myapp {
public:
    using config_type = std::tuple<>;
    static myapp create() { return {}; }
    template <auto World> // single world
    void run() {
        using namespace neutron;
        using namespace proton;
        using enum stage;

        // create allocator
        constexpr auto bufsize = 1024 * 1024 * 1;
        std::vector<std::byte> buf(bufsize);
        auto upstream     = std::pmr::monotonic_buffer_resource{};
        auto pool         = std::pmr::synchronized_pool_resource{ &upstream };
        using allocator_t = std::pmr::polymorphic_allocator<>;
        auto alloc        = allocator_t{ &pool };

        // create execution environment

        const auto concurrency = std::thread::hardware_concurrency();
        auto thread_pool =
            static_context_thread_pool<context_maker, allocator_t>{
                concurrency
            };
        execution::scheduler auto sch = thread_pool.get_scheduler();
        execution::sender auto sndr   = sch.schedule();
        // using command_buffer   = command_buffer<decltype(alloc)>;
        // auto command_buffers   = std::vector<
        //       command_buffer, rebind_alloc_t<decltype(alloc),
        //       command_buffer>>{
        //     32
        // };

        auto worlds = make_worlds<World>(alloc);
        // call_startup(worlds, sch);
        // call_update(worlds, sch);
        // call<shutdown>(worlds, sch);
    }
};
