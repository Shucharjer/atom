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
    archetype<> arche{ spread_type<std::string> };
    view<std::string> vw{ arche };
}

int main() {
    test_has();
    test_view();

    return 0;
}
