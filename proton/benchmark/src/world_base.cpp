#include <benchmark/benchmark.h>
#include <proton/args/common/commands.hpp>
#include <proton/command_buffer.hpp>
#include <proton/proton.hpp>
#include <proton/world_base.hpp>

using namespace proton;

template <>
constexpr bool as_component<int> = true;

using commands = basic_commands<>;

void BM_SpawnWithReserve(benchmark::State& state) {
    world_base<> world;
    world.reserve(state.range());
    for (auto _ : state) {
        for (auto i = 0; i < state.range(); ++i) {
            world.spawn();
        }
        world.clear();
    }
}

void BM_Spawn(benchmark::State& state) {
    world_base<> world;
    for (auto _ : state) {
        for (auto i = 0; i < state.range(); ++i) {
            world.spawn();
        }
        world.clear();
    }
}

void BM_SpawnCompWithReserve(benchmark::State& state) {
    world_base<> world;
    world.reserve(state.range());
    for (auto _ : state) {
        for (auto i = 0; i < state.range(); ++i) {
            world.spawn<int>();
        }
        world.clear();
    }
}

void BM_SpawnComp(benchmark::State& state) {
    world_base<> world;
    for (auto _ : state) {
        for (auto i = 0; i < state.range(); ++i) {
            world.spawn<int>();
        }
        world.clear();
    }
}

BENCHMARK(BM_Spawn)
    ->RangeMultiplier(10)
    ->Range(1000, 100000000)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_SpawnWithReserve)
    ->RangeMultiplier(10)
    ->Range(1000, 100000000)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_SpawnComp)
    ->RangeMultiplier(10)
    ->Range(1000, 10000000)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_SpawnCompWithReserve)
    ->RangeMultiplier(10)
    ->Range(1000, 10000000)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();
