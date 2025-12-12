#include <string>
#include <neutron/print.hpp>
#include <proton/archetype.hpp>
#include <proton/proton.hpp>

using namespace neutron;
using namespace proton;

template <>
constexpr inline bool as_component<std::string> = true;
template <>
constexpr inline bool as_component<int> = true;

void test_has() {
    archetype<> arche_string{ spread_type<std::string> };
    archetype<> arche_int{ spread_type<int> };
    archetype<> arche_both{ spread_type<int, std::string> };
    assert(arche_string.has<std::string>());
    assert(!arche_int.has<std::string>());
    assert(arche_both.has<std::string>() && arche_both.has<int>());
}

void test_view() {
    archetype<> arche{ spread_type<std::string, int> };
    arche.emplace(0, std::string{ "string" }, 32);
    arche.emplace(1, std::string{ "_1" }, 1);
    view<const std::string&, int> vw{ arche };
    println("before erase");
    for (auto [str, integer] : arche.view<const std::string&, int>()) {
        println("{}", str);
        println("{}", integer);
    }
    for (auto [integer, str] : arche.view<int, const std::string&>()) {
        println("{}", str);
        println("{}", integer);
    }
    arche.erase(0);
    println("after erase");
    for (auto [str, integer] : arche.view<const std::string&, int>()) {
        println("{}", str);
        println("{}", integer);
    }
    for (auto [integer, str] : arche.view<int, const std::string&>()) {
        println("{}", str);
        println("{}", integer);
    }
}

int main() {
    test_has();
    test_view();

    return 0;
}
