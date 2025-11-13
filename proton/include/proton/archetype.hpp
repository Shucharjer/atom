#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
#include "neutron/memory.hpp"
#include "neutron/neutron.hpp"
#include "neutron/type_hash.hpp"
#include "proton/proton.hpp"

namespace proton {

struct metatype {
    size_t size;
    size_t align;
    void (*construct)(void*);
    void (*destroy)(void*);

    template <typename Ty>
    consteval static metatype make() noexcept {
        return metatype{ .size      = std::is_empty_v<Ty> ? 0 : sizeof(Ty),
                         .align     = std::is_empty_v<Ty> ? 0 : alignof(Ty),
                         .construct = [](void* ptr) { ::new (ptr) Ty{}; },
                         .destroy = [](void* ptr) { static_cast<Ty*>(ptr)->~Ty(); } };
    }
};

template <typename... Tys>
consteval auto make_types() noexcept {
    return std::array{ metatype::make<Tys>()... };
}

template <_std_simple_allocator Alloc = std::allocator<std::byte>>
class archetype {
    template <typename Ty>
    using _rebind_alloc_t = rebind_alloc_t<Alloc, Ty>;

public:
    using allocator_type   = Alloc;
    using allocator_traits = std::allocator_traits<Alloc>;
    using size_type        = size_t;
    using difference_type  = ptrdiff_t;

    template <_comp_or_bundle... Components, typename Al = Alloc>
    constexpr archetype(neutron::type_spreader<Components>..., const Al& alloc);

    class chunk {
    public:
    private:
    };


    NODISCARD constexpr bool empty() const noexcept { return size_ != 0UL; }

    NODISCARD constexpr size_type size() const noexcept { return size_; }

    auto begin();

    auto end();

    template <typename... Args>
    NODISCARD bool has() const noexcept {
        if constexpr (sizeof...(Args) == 1U) {
            constexpr auto hash = neutron::hash_of<Args...>();
            return std::binary_search(hash_list_.begin(), hash_list_.end(), hash);
        } else {
            return (has<Args>() && ...);
        }
    }

private:
    std::vector<uint64_t, _rebind_alloc_t<uint64_t>> hash_list_;
    std::vector<metatype, _rebind_alloc_t<metatype>> metatypes_;

    size_t size_{};
    std::vector<chunk, _rebind_alloc_t<chunk>> chunks_;
};

#if HAS_CXX23

template <_std_simple_allocator Alloc>
template <_comp_or_bundle... Components, typename Al>
constexpr archetype<Alloc>::archetype(neutron::type_spreader<Components>..., const Al& alloc)
    : hash_list_(
          std::from_range, neutron::make_hash_array<neutron::type_list<Components...>>(), alloc),
      metatypes_(
          std::from_range,
          make_types<
              neutron::sorted_type_t<neutron::sorted_list_t<neutron::type_list<Components...>>>>(),
          alloc),
      chunks_(alloc) {}

#else

template <_std_simple_allocator Alloc>
template <_comp_or_bundle... Components, typename Al>
constexpr archetype<Alloc>::archetype(neutron::type_spreader<Components>..., const Al& alloc)
    : hash_list_(
          neutron::make_hash_array<neutron::type_list<Components...>>().begin(),
          neutron::make_hash_array<neutron::type_list<Components...>>().end(), alloc),
      metatypes_(
          make_types<
              neutron::sorted_type_t<neutron::sorted_list_t<neutron::type_list<Components...>>>>()
              .begin(),
          make_types<
              neutron::sorted_type_t<neutron::sorted_list_t<neutron::type_list<Components...>>>>()
              .end(),
          alloc),
      chunks_(alloc) {}

#endif

} // namespace proton
