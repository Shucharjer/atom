#include <string>
#include <neutron/print.hpp>
#include <proton/args/common/query.hpp>
#include <proton/world.hpp>
#include "neutron/type_hash.hpp"
#include "proton/proton.hpp"
#include "proton/world_accessor.hpp"

using namespace neutron;
using namespace proton;
using enum stage;

using name = std::string;
template <>
constexpr bool as_component<name> = true;

void hello_entity(query<with<const name&>> qry);

constexpr auto world_desc =
    ::proton::world_desc | add_system<update, &hello_entity>;

int main() {
    ::proton::world auto world = basic_world<registry<::world_desc>>{};
    auto& arches      = world_accessor::archetypes(world);
    auto [iter, succ] = arches.emplace(
        make_array_hash<type_list<std::string>>(), spread_type<std::string>);

    archetype<>& arche = iter->second;
    using query_t      = query<with<const std::string&>>;
    query_t qry{ world };

    println("before add entity");
    for (auto [e, name] : qry.get_with_entity()) {
        println("name: {}", name);
    }

    arche.emplace(0, std::string{ "added an entity" });
    qry = query_t{ world };

    println("after add entity");
    for (auto [e, name] : qry.get_with_entity()) {
        println("entity {} name: {}", e, name);
    }
    for (auto [name] : qry.get()) {
        println("name: {}", name);
    }

    return 0;
}
