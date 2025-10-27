#pragma once
#include <type_traits>
#include <neutron/type_list.hpp>
#include "proton/system.hpp"

namespace proton {

template <auto World>
struct world_traits {
    using desc_type = std::remove_cvref_t<decltype(World)>;
    using systems = neutron::type_list_filt_nvoid_t<system_list, desc_type>;
};

}