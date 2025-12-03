#pragma once
#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <memory>
#include <ranges>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
#include <neutron/ranges.hpp>
#include "neutron/auxiliary.hpp"
#include "neutron/shift_map.hpp"
#include "neutron/type_hash.hpp"
#include "proton/proton.hpp"

namespace proton {

struct metatype {
    uint64_t trivially_relocatible : 1;
    uint64_t _reserve : 15;
    uint64_t align : 16;
    uint64_t size : 32;

    void (*construct)(void*);
    void (*destroy)(void*);

    template <typename Ty>
    consteval static metatype make() noexcept {
        return metatype{ .trivially_copyable = std::is_trivially_copyable_v<Ty>,
                         ._reserve           = 0,
                         .lign      = std::is_empty_v<Ty> ? 0 : alignof(Ty),
                         .size      = std::is_empty_v<Ty> ? 0 : sizeof(Ty),
                         .construct = [](void* ptr) { ::new (ptr) Ty{}; },
                         .destroy =
                             [](void* ptr) { static_cast<Ty*>(ptr)->~Ty(); } };
    }
};

template <typename... Tys>
consteval auto make_types() noexcept {
    return std::array{ metatype::make<Tys>()... };
}

template <typename Alloc = std::allocator<std::byte>>
struct hive_access {

    template <typename Ty>
    consteval static hive_access make() noexcept {
        return hive_access<Alloc>{};
    }
};

template <typename... Ty>
consteval auto make_accesses() noexcept {}

template <_std_simple_allocator Alloc>
class archetype {
    template <typename Ty>
    using _allocator_t = rebind_alloc_t<Alloc, Ty>;

public:
    using _hash_type       = uint64_t;
    using _metatype        = metatype;
    using allocator_type   = _allocator_t<metatype>;
    using allocator_traits = std::allocator_traits<allocator_type>;
    using size_type        = size_t;
    using difference_type  = ptrdiff_t;

    template <typename Al = Alloc>
    constexpr archetype(const Al& alloc = {})
        : hash_list_(alloc), metatypes_(alloc) {}

    template <typename Al = Alloc>
    constexpr archetype(_hash_type hash, _metatype meta, const Al& alloc = {})
        : hash_list_{ hash, alloc }, metatypes_(meta, alloc) {}

    template <
        neutron::compatible_range<_hash_type> HashRng,
        neutron::compatible_range<_metatype> MetaRng, typename Al = Alloc>
    requires(std::ranges::sized_range<HashRng> ||
             std::ranges::sized_range<MetaRng>)
    constexpr archetype(
        HashRng&& hrange, MetaRng&& mrange, const Al& alloc = {})
        : hash_list_(_preset_size(hrange, mrange), alloc),
          metatypes_(_preset_size(hrange, mrange), alloc) {
        if constexpr (std::ranges::contiguous_range<HashRng>) {
            std::memcpy(
                hash_list_.data(), std::ranges::data(hrange),
                sizeof(_hash_type) * hash_list_.size());
        } else {
            std::ranges::copy(hrange, hash_list_.begin());
        }

        if constexpr (std::ranges::contiguous_range<MetaRng>) {
            std::memcpy(
                metatypes_.data(), std::ranges::data(mrange),
                sizeof(_metatype) * metatypes_.size());
        } else {
            std::ranges::copy(mrange, metatypes_.begin());
        }
    }

    template <
        neutron::compatible_range<_hash_type> HashRng,
        neutron::compatible_range<_metatype> MetaRng, typename Al = Alloc>
    requires(
        !std::ranges::sized_range<HashRng> &&
        !std::ranges::sized_range<MetaRng>)
    constexpr archetype(
        HashRng&& hrange, MetaRng&& mrange, const Al& alloc = {});

    template <component... Components, typename Al = Alloc>
    constexpr archetype(const Al& alloc = {});

    auto begin() {}

    auto end();

    template <typename... Args>
    NODISCARD bool has() const noexcept {
        if constexpr (sizeof...(Args) == 1U) {
            constexpr auto hash = neutron::hash_of<Args...>();
            return std::binary_search(
                this->hash_list_.begin(), this->hash_list_.end(), hash);
        } else {
            return (has<Args>() && ...);
        }
    }

    entity_t spawn() {
        //
        ++size_;
    }

    void kill(entity_t entity) {
        //
        --size_;
    }

    NODISCARD constexpr size_type kinds() const noexcept {
        return hash_list_.size();
    }

    NODISCARD constexpr size_type size() const noexcept { return size_; }

    NODISCARD constexpr bool empty() const noexcept { return size_ == 0UL; }

private:
    template <
        neutron::compatible_range<_hash_type> HashRng,
        neutron::compatible_range<_metatype> MetaRng>
    requires std::contiguous_iterator<std::ranges::iterator_t<HashRng>> &&
             (std::ranges::sized_range<HashRng> ||
              std::ranges::sized_range<MetaRng>)
    NODISCARD constexpr static size_type
        _preset_size(const HashRng& hrange, const MetaRng& mrange) noexcept {
        if constexpr (std::ranges::sized_range<HashRng>) {
            return std::ranges::size(hrange);
        } else {
            return std::ranges::size(mrange);
        }
    }

    std::vector<_hash_type, _allocator_t<_hash_type>> hash_list_;
    std::vector<_metatype, _allocator_t<_metatype>> metatypes_;
    size_t size_{};
};

template <_std_simple_allocator Alloc>
template <
    neutron::compatible_range<typename archetype<Alloc>::_hash_type> HashRng,
    neutron::compatible_range<typename archetype<Alloc>::_metatype> MetaRng,
    typename Al>
requires(!std::ranges::sized_range<HashRng> &&
         !std::ranges::sized_range<MetaRng>)
constexpr archetype<Alloc>::archetype(
    HashRng&& hrange, MetaRng&& mrange, const Al& alloc)
#if HAS_CXX23
    : hash_list_(std::from_range, std::forward<HashRng>(hrange), alloc),
      metatypes_(std::from_range, std::forward<MetaRng>(mrange), alloc){}
#else
    : hash_list_(hrange.begin(), hrange.end(), alloc),
      metatypes_(mrange.begin(), mrange.end(), alloc) {
}
#endif

} // namespace proton
