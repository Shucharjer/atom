#pragma once
#include <neutron/pipeline.hpp>
#include <neutron/template_list.hpp>
#include "proton/desc_list.hpp"
#include "proton/stage.hpp"

namespace proton {

template <typename...>
struct observer_list {};

template <stage Stage, auto Observer>
struct _observer {};

template <stage Stage, auto Observer>
constexpr inline add_staged_fn<observer_list, _observer, Stage, Observer> add_observer;

template <auto WorldDesc>
using extract_observers_t = neutron::type_list_filt_type_list_t<observer_list, desc_t<WorldDesc>>;

} // namespace proton
