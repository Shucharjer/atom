#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <memory>
#include <new>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
#include <neutron/algorithm.hpp>
#include <neutron/concepts.hpp>
#include <neutron/dense_map.hpp>
#include <neutron/ranges.hpp>
#include <neutron/shift_map.hpp>
#include <neutron/template_list.hpp>
#include <neutron/type_hash.hpp>
#include <neutron/utility.hpp>
#include "proton.hpp"
#include "proton/proton.hpp"

namespace proton {

struct basic_info {
    uint64_t trivially_copyable : 1;
    uint64_t trivially_relocatable : 1;
    uint64_t _reserve : 14;
    uint64_t align : 16;
    uint64_t size : 32;

    template <typename Ty>
    consteval static basic_info make() noexcept {
        return { .trivially_copyable    = std::is_trivially_copyable_v<Ty>,
                 .trivially_relocatable = neutron::trivially_relocatable<Ty>,
                 ._reserve              = 0,
                 .align                 = std::is_empty_v<Ty> ? 0 : alignof(Ty),
                 .size = std::is_empty_v<Ty> ? 0 : alignof(Ty) };
    }
};

template <typename... Tys>
consteval auto make_info() noexcept {
    return std::array{ basic_info::make<Tys>()... };
}

template <typename... Tys>
consteval auto make_info(neutron::type_list<Tys...>) noexcept {
    return std::array{ basic_info::make<Tys>()... };
}

template <component... Comp>
struct add_components_t {};
template <component... Comp>
struct remove_components_t {};

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays)
// NOLINTBEGIN(modernize-avoid-c-arrays)

struct _buffer_deletor {
    std::align_val_t align;
    constexpr void operator()(std::byte* ptr) const noexcept {
        ::operator delete(ptr, align);
    }
};

using _buffer_ptr = std::unique_ptr<std::byte[], _buffer_deletor>;

template <component... Components>
class view {
public:
    using value_type      = std::tuple<Components...>;
    using size_type       = size_t;
    using difference_type = ptrdiff_t;
    using _sorted_list    = neutron::type_list<Components...>;

    template <typename Archetype>
    constexpr view(Archetype& arche) noexcept
        : size_(arche.size_), storage_(/*TODO: init storage array*/) {}

    class iterator {
    public:
        using value_type      = std::tuple<Components...>;
        using difference_type = ptrdiff_t;

        auto operator*() {}
        auto operator++() {}
        auto operator++(int) {}
        auto operator+=(ptrdiff_t) {}
        auto operator+(ptrdiff_t) {}
        auto operator--() {}
        auto operator--(int) {}
        auto operator-=(ptrdiff_t) {}
        auto operator-(ptrdiff_t) {}

    private:
        size_type size_;
    };

    auto begin() noexcept {}
    auto end() noexcept {}

private:
    size_type size_;
    std::array<std::byte*, sizeof...(Components)> storage_;
};

template <_std_simple_allocator Alloc>
class archetype {
    template <component... Comp>
    friend class view;

    template <typename Ty>
    using _allocator_t = rebind_alloc_t<Alloc, Ty>;

    template <typename Ty>
    using _vector_t = std::vector<Ty, _allocator_t<Ty>>;

    template <
        typename Kty, typename Ty, typename Hash = std::hash<Kty>,
        typename Pred = std::equal_to<Kty>>
    using _unordered_map_t = std::unordered_map<
        Kty, Ty, Hash, Pred, _allocator_t<std::pair<const Kty, Ty>>>;

public:
    using _hash_type       = uint32_t;
    using allocator_type   = _allocator_t<std::byte>;
    using allocator_traits = std::allocator_traits<allocator_type>;
    using size_type        = size_t;
    using difference_type  = ptrdiff_t;

    template <component... Components, typename Al = Alloc>
    requires(sizeof...(Components) != 0)
    constexpr archetype(
        neutron::type_spreader<Components...>, const Al& alloc = {})
        : archetype(
              neutron::make_hash_array<neutron::type_list<Components...>>(),
              make_info(
                  neutron::sorted_type_t<neutron::sorted_list_t<
                      neutron::type_list<Components...>>>{}),
              alloc) {}

    template <
        neutron::compatible_range<_hash_type> HashRng,
        neutron::compatible_range<basic_info> InfoRng, typename Al = Alloc>
    constexpr archetype(
        HashRng&& hrange, InfoRng&& irange, const Al& alloc = {})
        : hash_list_(
              std::ranges::begin(hrange), std::ranges::end(hrange), alloc),
          basic_info_(
              std::ranges::begin(irange), std::ranges::end(irange), alloc),
          hash_(neutron::hash_combine(hash_list_)) {}

    template <
        neutron::compatible_range<_hash_type> HashRng,
        neutron::compatible_range<basic_info> InfoRng, typename... Components,
        typename Al = Alloc>
    constexpr archetype(
        HashRng&& hrange, InfoRng&& irange, add_components_t<Components...>,
        const Al& alloc = {})
        : hash_list_(
              std::ranges::begin(hrange), std::ranges::end(hrange), alloc),
          basic_info_(
              std::ranges::begin(irange), std::ranges::end(irange), alloc) {
        using type_list          = neutron::type_list<Components...>;
        constexpr auto hash_list = neutron::make_hash_array<type_list>();

        // TODO: append info, constructors, move constructors, destructors
        static_assert(false);
#if HAS_CXX23
        hash_list_.append_range(hash_list);
#else
        hash_list_.insert(hash_list_.end(), hash_list.begin(), hash_list.end());
#endif
        neutron::inplace_merge(
            hash_list_, hash_list_.end() - hash_list.size(), basic_info_,
            constructors_, move_constructors_, destructors_);
    }

    template <
        neutron::compatible_range<_hash_type> HashRng,
        neutron::compatible_range<basic_info> InfoRng, typename... Components,
        typename Al = Alloc>
    constexpr archetype(
        HashRng&& hrange, InfoRng&& irange, remove_components_t<Components...>,
        const Al& alloc = {})
        : hash_list_(
              std::ranges::begin(hrange), std::ranges::end(hrange), alloc),
          basic_info_(
              std::ranges::begin(irange), std::ranges::end(irange), alloc) {
        // TODO: remove hash, info, constructors, move constructors and
        // destructors and sort them
        static_assert(false);
    }

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

    NODISCARD constexpr auto emplace(entity_t entity) { _emplace(entity); }

    template <component... Components>
    NODISCARD constexpr auto emplace(entity_t entity) {
        constexpr uint64_t combined_hash =
            neutron::make_array_hash<neutron::type_list<Components...>>();
        assert(combined_hash == hash_);

        emplace(entity);
    }

    template <component... Components>
    NODISCARD constexpr auto
        emplace(entity_t entity, Components&&... components) {
        constexpr uint64_t combined_hash = neutron::make_array_hash<
            neutron::type_list<std::remove_cvref_t<Components>...>>();
        assert(combined_hash == hash_);

        _emplace(entity, std::forward<Components>(components)...);
    }

    constexpr void erase(entity_t entity) noexcept {
        const auto index = entity2index_.at(entity);
        for (auto i = 0; i < hash_list_.size(); ++i) {
            const basic_info info = basic_info_[i];
            _buffer_ptr& data     = storage_[i];
            destructors_[i](data.get() + (index * info.size));
        }
        index2entity_[index]                   = index2entity_.back();
        entity2index_.at(index2entity_[index]) = index;
        entity2index_.erase(entity);
        --size_;
    }

    NODISCARD constexpr size_type kinds() const noexcept {
        return hash_list_.size();
    }

    NODISCARD constexpr size_type size() const noexcept { return size_; }

    NODISCARD constexpr bool empty() const noexcept { return size_ == 0UL; }

    NODISCARD constexpr size_type capacity() const noexcept {
        return capacity_;
    }

    NODISCARD constexpr decltype(auto) hash_list() const noexcept {
        return hash_list_;
    }

private:
    template <
        neutron::compatible_range<_hash_type> HashRng,
        neutron::compatible_range<basic_info> InfoRng>
    requires std::contiguous_iterator<std::ranges::iterator_t<HashRng>> &&
             (std::ranges::sized_range<HashRng> ||
              std::ranges::sized_range<InfoRng>)
    NODISCARD constexpr static size_type
        _preset_size(const HashRng& hrange, const InfoRng& irange) noexcept {
        if constexpr (std::ranges::sized_range<HashRng>) {
            return std::ranges::size(hrange);
        } else {
            return std::ranges::size(irange);
        }
    }

    constexpr auto _emplace(entity_t entity) {
        const index_t index = size_;
        if (size_ != capacity_) [[likely]] {
            _emplace_normally();
        } else [[unlikely]] {
            _emplace_with_relocate();
        }
        entity2index_.try_emplace(entity, index);
        index2entity_.push_back(entity);
        ++size_;
    }

    constexpr auto _emplace_normally() {
        for (auto i = 0; i < hash_list_.size(); ++i) {
            const basic_info info = basic_info_[i];
            _buffer_ptr& data     = storage_[i];
            auto* const ptr       = data.get();
            constructors_[i](ptr + (size_ * info.size));
        }
    }

    constexpr auto _emplace_with_relocate() {
        const auto new_capacity = capacity_ << 1;
        for (auto i = 0; i < hash_list_.size(); ++i) {
            const basic_info info = basic_info_[i];
            const auto align =
                std::align_val_t{ (std::max<size_type>)(32, info.align) };
            const _buffer_deletor deletor{ align };

            auto& data = storage_[i];
            auto* ptr  = static_cast<std::byte*>(
                ::operator new(new_capacity * info.size, align));
            if (info.trivially_relocatable) {
                std::memcpy(
                    std::assume_aligned<32>(ptr),
                    std::assume_aligned<32>(storage_[i].get()),
                    capacity_ * info.size);
            } else {
                for (auto i = 0; i < size_; ++i) {
                    const ptrdiff_t diff =
                        static_cast<ptrdiff_t>(info.size) * i;
                    auto* const src = data.get() + diff;
                    move_constructors_[i](ptr + diff, src);
                    destructors_[i](src);
                }
            }
            constructors_[i](ptr + (capacity_ * info.size));
            data = _buffer_ptr{ ptr, deletor };
        }
        capacity_ = new_capacity;
    }

    template <component... Components>
    constexpr void _emplace(entity_t entity, Components&&... components) {
        using namespace neutron;
        using sorted =
            sorted_type_t<type_list<std::remove_cvref_t<Components>...>>;
        _emplace<sorted>(
            entity,
            std::forward_as_tuple(std::forward<Components>(components)...));
    }

    template <component... SortedComponents, typename Tup>
    constexpr void _emplace(
        entity_t entity,
        [[maybe_unused]] neutron::type_list<SortedComponents...>,
        Tup&& components) {
        if (size_ != capacity_) [[likely]] {
            _emplace_normally(std::forward<Tup>(components));
        } else [[unlikely]] {
            _emplace_with_relocate(std::forward<Tup>(components));
        }
        ++size_;
    }

    template <component... SortedComponents, typename Tup>
    constexpr void _emplace_normally(
        [[maybe_unused]] std::tuple<SortedComponents...>, Tup&& components) {
        using sorted_list = std::tuple<SortedComponents...>;
        [this, tup = std::forward<Tup>(components)]<size_t... Is>(
            std::index_sequence<Is...>) {
            ((::new (
                 storage_[Is].get() +
                 (size_ * sizeof(std::tuple_element_t<Is, sorted_list>)))
                  std::tuple_element_t<Is, sorted_list>(
                      std::get<std::tuple_element_t<Is, Tup>>(tup))),
             ...);
        }(std::index_sequence_for<SortedComponents...>());
    }

    template <size_t Index, typename Tup, typename FwdTup>
    void _emplace_with_relocate(size_type new_capacity, FwdTup&& tup) {
        using type = std::tuple_element_t<Index, Tup>;
        constexpr auto align =
            std::align_val_t{ (std::max<size_t>)(32, alignof(type)) };

        _buffer_ptr& data = storage_[Index];
        auto* ptr         = static_cast<std::byte*>(
            ::operator new(sizeof(type) * new_capacity, align));
        if constexpr (neutron::trivially_relocatable<type>) {
            std::memcpy(
                std::assume_aligned<align>(ptr),
                std::assume_aligned<align>(data.get()), capacity_);
        } else {
            auto* const input  = reinterpret_cast<type*>(data.get());
            auto* const output = reinterpret_cast<type*>(ptr);
            std::uninitialized_move_n(input, size_, output);
        }
        ::new (ptr) type(std::get<type>(std::forward<FwdTup>(tup)));
        data = _buffer_ptr{ ptr, _buffer_deletor{ align } };
    }

    template <component... SortedComponents, typename Tup>
    constexpr void _emplace_with_relocate(
        [[maybe_unused]] neutron::type_list<SortedComponents...>,
        Tup&& components) {
        using sorted_list       = std::tuple<SortedComponents...>;
        const auto new_capacity = capacity_ << 1;
        [this, tup = std::forward<Tup>(components)]<size_t... Is>(
            std::index_sequence<Is...>) {
            (_emplace_with_relocate<Is, sorted_list>(
                 new_capacity, std::forward<Tup>(tup)),
             ...);
        }(std::index_sequence_for<SortedComponents...>());
        capacity_ = new_capacity;
    }

    _vector_t<_hash_type> hash_list_;
    _vector_t<basic_info> basic_info_;
    _vector_t<void (*)(void*)> constructors_;
    _vector_t<void (*)(void*, void*)> move_constructors_;
    _vector_t<void (*)(void*)> destructors_;
    _vector_t<_buffer_ptr> storage_;
    size_type size_{};
    size_type capacity_{};
    uint64_t hash_{};
    neutron::shift_map<
        entity_t, index_t, 32UL, neutron::half_bits<entity_t>, Alloc>
        entity2index_;
    _vector_t<entity_t> index2entity_;
};

// NOLINTEND(modernize-avoid-c-arrays)
// NOLINTEND(cppcoreguidelines-avoid-c-arrays)
// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

} // namespace proton
