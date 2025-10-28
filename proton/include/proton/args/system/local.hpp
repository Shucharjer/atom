#pragma once
#include <tuple>

namespace proton {

template <typename... Args>
struct local {
    std::tuple<Args&...> value;
};

template <auto System, typename... Args>
struct _local_desc {};

} // namespace proton
