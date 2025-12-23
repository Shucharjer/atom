#include <benchmark/benchmark.h>
#include <proton/args/common/commands.hpp>
#include <proton/command_buffer.hpp>
#include <proton/proton.hpp>
#include <proton/world_base.hpp>

using namespace proton;

template <>
constexpr bool as_component<int> = true;
template <>
constexpr bool as_component<std::string> = true;

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

void BM_SpawnOneCompWithReserve(benchmark::State& state) {
    world_base<> world;
    world.reserve<int>(state.range());
    for (auto _ : state) {
        for (auto i = 0; i < state.range(); ++i) {
            world.spawn<int>();
        }
        world.clear();
    }
}

void BM_SpawnOneComp(benchmark::State& state) {
    world_base<> world;
    for (auto _ : state) {
        for (auto i = 0; i < state.range(); ++i) {
            world.spawn<int>();
        }
        world.clear();
    }
}

void BM_SpawnTwoCompWithReserve(benchmark::State& state) {
    world_base<> world;
    world.reserve<int, std::string>(state.range());
    for (auto _ : state) {
        for (auto i = 0; i < state.range(); ++i) {
            world.spawn<int, std::string>();
        }
        world.clear();
    }
}

void BM_SpawnTwoComp(benchmark::State& state) {
    world_base<> world;
    for (auto _ : state) {
        for (auto i = 0; i < state.range(); ++i) {
            world.spawn<int, std::string>();
        }
        world.clear();
    }
}

void BM_SpawnAndKill(benchmark::State& state) {
    world_base<> world;
    for (auto _ : state) {
        for (auto i = 0; i < state.range(); ++i) {
            const auto entity = world.spawn();
            world.kill(entity);
        }
    }
}

BENCHMARK(BM_Spawn)
    ->RangeMultiplier(10)
    ->Range(1, 100000000)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_SpawnWithReserve)
    ->RangeMultiplier(10)
    ->Range(1, 100000000)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_SpawnOneComp)
    ->RangeMultiplier(10)
    ->Range(1, 10000000)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_SpawnOneCompWithReserve)
    ->RangeMultiplier(10)
    ->Range(1, 10000000)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_SpawnTwoComp)
    ->RangeMultiplier(10)
    ->Range(1, 10000000)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_SpawnTwoCompWithReserve)
    ->RangeMultiplier(10)
    ->Range(1, 10000000)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_SpawnAndKill)
    ->RangeMultiplier(10)
    ->Range(1, 10000000)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();
