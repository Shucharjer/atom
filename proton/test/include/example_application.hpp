#pragma once
#include <cstddef>
#include <memory_resource>
#include <tuple>
#include <vector>
#include <neutron/thread_pool.hpp>
#include <proton/execution.hpp>
#include <proton/registry.hpp>
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
        // constexpr auto bufsize = 1024 * 1024 * 1;
        // std::vector<std::byte> buf(bufsize);
        // auto upstream      = std::pmr::monotonic_buffer_resource{};
        // auto pool          = std::pmr::synchronized_pool_resource{ &upstream
        // }; auto alloc         = std::pmr::polymorphic_allocator<>{ &pool };
        // scheduler auto sch = thread_pool.scheduler();
        auto worlds = make_worlds<World>();
        call_startup(worlds);
        call_update(worlds);
        call<shutdown>(worlds);
    }
};
