#pragma once
#include <cstdint>
#include <type_traits>
#include <unordered_map>
#include "neutron/reflection.hpp"

namespace proton {

struct metatype {
    size_t hash;
    size_t size;
    size_t align;
    void (*construct)(void*);
    void (*destroy)(void*);
};

template <typename Ty>
consteval metatype make_type() noexcept {
    return metatype{ .hash      = neutron::hash_of<Ty>(),
                     .size      = std::is_empty_v<Ty> ? 0 : sizeof(Ty),
                     .align     = std::is_empty_v<Ty> ? 0 : alignof(Ty),
                     .construct = [](void* ptr) { ::new (ptr) Ty{}; },
                     .destroy = [](void* ptr) { static_cast<Ty*>(ptr)->~Ty(); } };
}

class archetype {
public:
private:
    uint64_t hash_;
    
};

} // namespace proton
