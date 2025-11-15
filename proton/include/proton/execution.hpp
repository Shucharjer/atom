#pragma once
#include <version>

#if defined(__cpp_lib_execution) && __cpp_lib_execution >= 202602L
    #include <execution>
#elif __has_include(<stdexec/execution.hpp>)
    #include <stdexec/execution.hpp>
#else
    #include <concepts>
    #include <exception>
    #include <type_traits>
    #include <utility>
    #include "neutron/internal/tag_invoke.hpp"
#endif

namespace proton::execution {

#if defined(__cpp_lib_execution) && __cpp_lib_execution >= 202602L

using namespace std::execution;

#elif __has_include(<stdexec/execution.hpp>)

using namespace stdexec;

#else

/*

provides type:
receiver_t, sender_t, success_t;

privides cpo (by tag_invoke):
get_env, connect, get_completion_signaures, start, on, then, sync_wait.

*/

struct receiver_t {};

template <typename Receiver>
concept as_receivable =
    (requires { typename Receiver::receiver_concept; } &&
     std::derived_from<typename Receiver::receiver_concept, receiver_t>) ||
    requires { typename Receiver::is_receiver; };

template <typename Queryable>
concept queryable = std::destructible<Queryable>;

struct get_env_t {
    template <typename EnvProvider>
    requires tag_invocable<get_env_t, const EnvProvider&>
    constexpr auto operator()(const EnvProvider ep) const noexcept
        -> tag_invoke_result_t<get_env_t, const EnvProvider&> {
        return tag_invoke(*this, ep);
    }
};

constexpr inline get_env_t get_env;

template <typename EnvProvider>
concept environment_provider = requires(EnvProvider& provider) {
    { get_env(std::as_const(provider)) } -> queryable;
};

template <typename Receiver>
concept receiver = as_receivable<Receiver> && environment_provider<Receiver> &&
                   std::move_constructible<Receiver> &&
                   std::constructible_from<std::decay_t<Receiver>, Receiver>;

struct sender_t {};

template <typename Sndr>
concept as_sendable =
    std::derived_from<typename Sndr::sender_concept, sender_t> ||
    requires { typename Sndr::is_sender; }
    // false
    || std::same_as<void, void*> /* awaitable */;

template <typename Sndr>
concept sender =
    as_sendable<std::decay_t<Sndr>> && environment_provider<Sndr> &&
    std::move_constructible<std::decay_t<Sndr>> &&
    std::constructible_from<std::decay_t<Sndr>, Sndr>;

template <typename Rcvr>
using env_of_t = std::invoke_result_t<get_env_t, Rcvr>;

using success_t = int;

template <typename... Sigs>
struct completion_signatures {};

/*! @cond TURN_OFF_DOXYGEN */
namespace _completion_signature {

template <typename>
constexpr bool is_completion_signatures = false;

template <typename... Sigs>
constexpr bool is_completion_signatures<completion_signatures<Sigs...>> = true;

template <typename Completions>
concept valid_completion_signatures =
    std::convertible_to<Completions, success_t> &&
    is_completion_signatures<Completions>;

} // namespace _completion_signature
/*! @endcond */

struct empty_env {};

struct get_completion_signatures_t {
    template <typename Sndr, typename Env = empty_env>
    constexpr auto operator()(Sndr&&, Env&& = {}) const noexcept
        -> decltype(2) {
        return {};
    }
};

namespace internal {

template <typename Rcvr, typename Tag, typename... Args>
auto _try_completion(Tag (*)(Args...)) -> int {}

} // namespace internal

template <typename Rcvr, typename Completions>
concept receiver_of = receiver<Rcvr> && requires(Completions* comp) {
    {
        try_completions<std::decay_t<Rcvr>>(comp)
    } -> std::convertible_to<success_t>;
};

using _completion_signature::valid_completion_signatures;

template <typename Sndr, typename... Env>
concept sender_in =
    (sizeof...(Env) <= 1) && sender<Sndr> &&
    requires(Sndr&& sndr, Env&&... env) {
        {
            get_completion_signatures(
                std::forward<Sndr>(sndr), std::forward<Env>(env)...)
        } -> valid_completion_signatures;
    };

struct connect_t;

namespace _connect {

template <typename Sndr, typename Rcvr>
concept connectable_with_tag_invoke =
    receiver<Rcvr> && sender_in<Sndr, env_of_t<Rcvr>> &&
    receiver_of<Rcvr, Sndr> && tag_invocable<connect_t, Sndr, Rcvr>;

template <typename Sndr, typename Rcvr>
concept connectable_with_co_await = requires { false; };

} // namespace _connect

struct connect_t {
    template <sender Sndr, receiver Rcvr>
    requires _connect::connectable_with_tag_invoke<Sndr, Rcvr> ||
             _connect::connectable_with_co_await<Sndr, Rcvr>
    constexpr auto operator()(Sndr&& sndr, Rcvr&& rcvr) const {
        auto&& env  = get_env(rcvr);
        auto domain = get_late_domain(sndr, rcvr);

        if constexpr (_connect::connectable_with_tag_invoke<Sndr, Rcvr>) {
            return tag_invoke(
                connect_t{},
                transform_sender(domain, std::forward<Sndr>(sndr), env),
                std::forward<Rcvr>(rcvr));
        } else if constexpr (_connect::connectable_with_co_await<Sndr, Rcvr>) {
            return connect_awaitable(
                transform_sender(domain, std::forward<Sndr>(sndr), env),
                std::forward<Rcvr>(rcvr));
        }
    }
};

constexpr inline connect_t connect{};

struct _sync_wait_t {
    template <typename _, sender Sndr>
    constexpr decltype(auto) operator()(_&&, Sndr&& sndr) {}
};

constexpr inline _sync_wait_t sync_wait{};

template <typename Scheduler>
concept scheduler = true;

template <typename Rndr, typename Fn>
struct _then_receiver {};

struct set_error_t {};

constexpr inline set_error_t set_error{};

struct set_value_t {};

constexpr inline set_value_t set_value{};

template <sender Sndr, typename Fn>
class _then_sender {
public:
    using is_sender = void;

private:
    template <typename Env>
    friend auto tag_invoke(
        get_completion_signatures_t, _then_sender&& self, Env)
        -> make_completion_signatures<
            Sndr, Env, completion_signatures<set_error_t(std::exception_ptr)>,
            set_value_t>;

    template <receiver Rcvr>
    friend auto tag_invoke(connect_t, _then_sender&& self, Rcvr rcvr)
        -> connect_result_t<Sndr, _then_sender<Sndr, Fn>> {}

    Sndr sndr_;
    Fn fn_;
};

#endif

} // namespace proton::execution
