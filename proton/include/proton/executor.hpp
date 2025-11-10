#pragma once
#include <atomic>
#include <latch>
#include <ranges>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>
#include "neutron/template_list.hpp"

namespace proton {

class single_task_executor {
    template <typename, typename... Args>
    class future;

    template <template <auto...> typename Template, auto... Fn, typename... Args>
    class future<Template<Fn...>, Args...> {
        std::tuple<Args...> args_;

    public:
        constexpr future(Args&... args) noexcept(
            std::is_nothrow_constructible_v<std::tuple<Args...>, Args...>)
            : args_(args...) {}

        void wait() { (std::apply(Fn, args_), ...); }
    };

    template <typename... Fn, typename... Args>
    class future<std::tuple<Fn...>, Args...> {
        std::tuple<Fn...> fn_;
        std::tuple<Args...> args_;

    public:
        template <typename Tup>
        constexpr future(Tup& fn, Args... args) : fn_(fn), args_(args...) {}

        void wait() {
            [this]<size_t... Is>(std::index_sequence<Is...>) {
                (std::apply(std::get<Is>(fn_), args_), ...);
            }(std::index_sequence_for<Fn...>());
        }
    };

    template <std::ranges::range Rng, typename... Args>
    class future<Rng, Args...> {
        Rng range_;
        std::tuple<Args...> args_;

    public:
        template <std::ranges::range R>
        constexpr future(R&& range, Args... args)
            : range_(std::forward<Rng>(range)), args_(args...) {}

        void wait() {
            std::ranges::for_each(range_, [this](auto fn) { std::apply(fn, args_); });
        }
    };

public:
    template <auto... Fn, typename... Args>
    constexpr auto submit(Args&... args) const noexcept(
        std::is_nothrow_constructible_v<future<neutron::value_list<Fn...>, Args...>, Args...>) {
        return future<neutron::value_list<Fn...>, Args...>(args...);
    }

    template <typename FnTuple, typename... Args>
    constexpr auto submit(FnTuple&& fn, Args&&... args) const noexcept(
        std::is_nothrow_constructible_v<future<FnTuple, Args...>, Args...>) {
        return future(std::forward<FnTuple>(fn), std::forward<Args>(args)...);
    }

    template <std::ranges::range Rng, typename... Args>
    constexpr auto submit(Rng&& range, Args&&... args) const {
        return future(std::forward<Rng>(range), std::forward<Args>(args)...);
    }
};

class basic_executor {
public:
    template <auto... Fn, typename... Args>
    auto submit(Args&&... args);

    template <typename FnTuple, typename... Args>
    constexpr auto submit(FnTuple&& fn, Args&&... args);

    template <std::ranges::range Rng, typename... Args>
    constexpr auto submit(Rng&& range, Args&&... args);

private:
};

} // namespace proton
