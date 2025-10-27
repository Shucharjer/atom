#pragma once
#include <cstdint>

namespace proton {

enum class stage : uint8_t {
    pre_startup,
    startup,
    post_startup,
    first,
    pre_update,
    update,
    post_update,
    render,
    last,
    shutdown
};

} // namespace proton
