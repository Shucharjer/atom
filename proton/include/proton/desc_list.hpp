#pragma once
#include <neutron/pipeline.hpp>
#include <neutron/template_list.hpp>
#include "proton/stage.hpp"
#include "proton/world_desc.hpp"

namespace proton {

template <
    template <typename...> typename Template,
    template <auto, typename...> typename Fn, auto Value, typename... Requires>
struct add_fn :
    neutron::adaptor_closure<add_fn<Template, Fn, Value, Requires...>> {
    using input_require  = world_require<void>;
    using output_type    = world_descriptor<>;
    using consteval_pipe = void;

    template <typename WorldDesc>
    constexpr auto operator()(WorldDesc&&) const noexcept {
        return neutron::insert_type_list_inplace_t<
            Template, Fn<Value, Requires...>, std::remove_cvref_t<WorldDesc>>{};
    }
};

template <
    template <typename...> typename Template,
    template <stage, auto, typename...> typename Fn, stage Stage, auto Value,
    typename... Requires>
struct add_staged_fn :
    neutron::adaptor_closure<
        add_staged_fn<Template, Fn, Stage, Value, Requires...>> {
    using input_require  = world_require<void>;
    using output_type    = world_descriptor<>;
    using consteval_pipe = void;

    template <typename WorldDesc>
    constexpr auto operator()(WorldDesc&&) const noexcept {
        return neutron::insert_type_list_inplace_t<
            Template, Fn<Stage, Value, Requires...>,
            std::remove_cvref_t<WorldDesc>>{};
    }
};

template <stage Stage, typename Desc>
struct in_stage : std::false_type {};
template <
    stage Stage, template <stage, auto, typename...> typename Fn, auto System,
    typename... Requires>
struct in_stage<Stage, Fn<Stage, System, Requires...>> : std::true_type {};

template <typename Desc>
using in_pre_startup_stage = in_stage<stage::pre_startup, Desc>;
template <typename Desc>
using in_startup_stage = in_stage<stage::startup, Desc>;
template <typename Desc>
using in_post_startup_stage = in_stage<stage::post_startup, Desc>;
template <typename Desc>
using in_first_stage = in_stage<stage::first, Desc>;
template <typename Desc>
using in_pre_update_stage = in_stage<stage::pre_update, Desc>;
template <typename Desc>
using in_update_stage = in_stage<stage::update, Desc>;
template <typename Desc>
using in_post_update_stage = in_stage<stage::post_update, Desc>;
template <typename Desc>
using in_render_stage = in_stage<stage::render, Desc>;
template <typename Desc>
using in_last_stage = in_stage<stage::last, Desc>;
template <typename Desc>
using in_shutdown_stage = in_stage<stage::shutdown, Desc>;

} // namespace proton
