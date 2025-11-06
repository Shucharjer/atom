#pragma once
#include <type_traits>
#include "neutron/value_list.hpp"

namespace proton {

class single_task_executor {
    template <typename, typename... Args>
    class future;
    template <template <auto...> typename Template, auto... Fn, typename... Args>
    class future<Template<Fn...>, Args...> {
        std::tuple<Args...> args_;
        constexpr future(Args&... args) noexcept(
            std::is_nothrow_constructible_v<std::tuple<Args...>, Args...>)
            : args_(args...) {}

    public:
        void wait() { (std::apply(Fn, args_), ...); }
    };

public:
    template <auto... Fn, typename... Args>
    constexpr auto submit(Args&... args) {
        return future<neutron::value_list<Fn...>, Args...>(args...);
    }
};

class basic_executor {
public:
    template <auto... Fn, typename... Args>
    auto submit(Args&... args) {}

    void wait() const noexcept {}

private:
};

} // namespace proton
