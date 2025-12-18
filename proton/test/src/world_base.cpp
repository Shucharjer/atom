#include <chrono>
#include <string>
#include <neutron/shift_map.hpp>
#include <proton/proton.hpp>
#include <proton/world_base.hpp>
#include "neutron/print.hpp"

using namespace std::chrono;
using namespace neutron;
using namespace proton;

template <>
constexpr inline bool as_component<std::string> = true;

constexpr auto loop = 8000'0000;

void test_vector_with_reserve() {
    std::vector<int> vec;
    vec.reserve(loop);

    auto beg = high_resolution_clock::now();
    for (auto i = 0; i < loop; ++i) {
        vec.emplace_back(i);
    }
    auto end     = high_resolution_clock::now();
    auto elapsed = duration_cast<microseconds>(end - beg);
    println(
        "vector emplaced {} times in {} microseconds", loop, elapsed.count());
}

void test_unordered_map_with_reserve() {
    std::unordered_map<entity_t, void*> map;
    map.reserve(loop);

    auto beg = high_resolution_clock::now();
    for (auto i = 0; i < loop; ++i) {
        map.emplace(i, nullptr);
    }
    auto end     = high_resolution_clock::now();
    auto elapsed = duration_cast<microseconds>(end - beg);
    println(
        "unordered_map emplaced {} times in {} microseconds", loop,
        elapsed.count());
}

void test_shift_map_with_reserve() {
    shift_map<entity_t, void*, 32UL> map;
    map.reserve(loop);

    auto beg = high_resolution_clock::now();
    for (auto i = 0; i < loop; ++i) {
        map.try_emplace(i, nullptr);
    }
    auto end     = high_resolution_clock::now();
    auto elapsed = duration_cast<microseconds>(end - beg);
    println(
        "shift_map emplaced {} times in {} microseconds", loop,
        elapsed.count());
}

void test_emplace_with_reserve() {
    world_base<> world;
    world.reserve(loop);
    auto beg = high_resolution_clock::now();
    for (auto i = 0; i < loop; ++i) {
        world.spawn();
    }
    auto end     = high_resolution_clock::now();
    auto elapsed = duration_cast<microseconds>(end - beg);
    println(
        "world emplaced {} times in {} microseconds", loop, elapsed.count());
}

void test_vector() {
    std::vector<int> vec;

    auto beg = high_resolution_clock::now();
    for (auto i = 0; i < loop; ++i) {
        // NOLINTNEXTLINE(performance-inefficient-vector-operation)
        vec.emplace_back(i);
    }
    auto end     = high_resolution_clock::now();
    auto elapsed = duration_cast<microseconds>(end - beg);
    println(
        "vector emplaced {} times in {} microseconds", loop, elapsed.count());
}

void test_unordered_map() {
    std::unordered_map<entity_t, void*> map;

    auto beg = high_resolution_clock::now();
    for (auto i = 0; i < loop; ++i) {
        map.emplace(i, nullptr);
    }
    auto end     = high_resolution_clock::now();
    auto elapsed = duration_cast<microseconds>(end - beg);
    println(
        "unordered_map emplaced {} times in {} microseconds", loop,
        elapsed.count());
}

void test_shift_map() {
    shift_map<entity_t, void*, 32UL> map;

    auto beg = high_resolution_clock::now();
    for (auto i = 0; i < loop; ++i) {
        map.try_emplace(i, nullptr);
    }
    auto end     = high_resolution_clock::now();
    auto elapsed = duration_cast<microseconds>(end - beg);
    println(
        "shift_map emplaced {} times in {} microseconds", loop,
        elapsed.count());
}

void test_emplace() {
    world_base<> world;
    auto beg = high_resolution_clock::now();
    for (auto i = 0; i < loop; ++i) {
        world.spawn();
    }
    auto end     = high_resolution_clock::now();
    auto elapsed = duration_cast<microseconds>(end - beg);
    println(
        "world emplaced {} times in {} microseconds", loop, elapsed.count());
}

int main() {
    test_vector_with_reserve();
    test_unordered_map_with_reserve();
    test_shift_map_with_reserve();
    test_emplace_with_reserve();

    test_vector();
    test_unordered_map();
    test_shift_map();
    test_emplace();

    return 0;
}
