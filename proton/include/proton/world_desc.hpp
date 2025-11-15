#pragma once
#include <tuple>
#include <type_traits>
#include <neutron/pipeline.hpp>

namespace proton {

template <typename... Lists>
struct world_descriptor {};

constexpr inline world_descriptor<> world_desc;

template <typename>
struct world_require : std::false_type {};
template <typename... Lists>
struct world_require<world_descriptor<Lists...>> : std::true_type {};
template <typename... Lists>
struct world_require<world_descriptor<Lists...>&> : std::true_type {};
template <typename... Lists>
struct world_require<const world_descriptor<Lists...>&> : std::true_type {};
template <typename... Lists>
struct world_require<world_descriptor<Lists...>&&> : std::true_type {};

template <typename Ty>
struct application_require {
    constexpr static bool value = requires {
        typename Ty::config_type; // tuple
    } && (requires (Ty& obj) {
        obj.template run<world_desc>();
    } || requires (Ty& obj) {
        obj.template run<world_desc>(std::declval<typename Ty::config_type>());
    });
};

template <typename Tuple, auto... Worlds>
class run_worlds_fn;

template <auto... Worlds>
class run_worlds_fn<std::tuple<>, Worlds...> :
    public neutron::adaptor_closure<run_worlds_fn<std::tuple<>, Worlds...>> {

public:
    using input_require = application_require<void>;
    using output_type   = void;

    constexpr run_worlds_fn() noexcept = default;

    template <typename Application>
    void operator()(Application&& application) {
        std::forward<Application>(application).template run<Worlds...>();
    }
};

template <typename... Args, auto... Worlds>
class run_worlds_fn<std::tuple<Args...>, Worlds...> :
    public neutron::adaptor_closure<
        run_worlds_fn<std::tuple<Args...>, Worlds...>> {
    using tuple_type = std::tuple<Args...>;
    std::tuple<Args...> tup_;

public:
    using input_require = application_require<void>;
    using output_type   = void;

    template <typename... ArgTys>
    constexpr run_worlds_fn(ArgTys&&... args) noexcept(
        std::is_nothrow_constructible_v<tuple_type, ArgTys...>)
        : tup_(std::forward<ArgTys>(args)...) {}

    template <typename Application>
    constexpr auto operator()(Application&& application) {
        return std::forward<Application>(application)
            .template run<Worlds...>(std::move(tup_));
    }
};

template <auto... Worlds>
struct _run_worlds_wrapper {
    template <typename... Args>
    constexpr auto operator()(Args&&... args) const noexcept(
        std::is_nothrow_constructible_v<std::tuple<Args...>, Args...>) {
        return run_worlds_fn<std::tuple<Args...>, Worlds...>(
            std::forward<Args>(args)...);
    }
};

template <auto... Worlds>
constexpr inline _run_worlds_wrapper<Worlds...> run_worlds;

template <auto WorldDesc>
using desc_t = std::remove_cvref_t<decltype(WorldDesc)>;

} // namespace proton
