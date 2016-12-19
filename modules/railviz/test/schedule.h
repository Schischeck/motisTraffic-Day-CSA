#pragma once

#include "motis/loader/loader_options.h"

namespace motis {
namespace railviz {

static loader::loader_options dataset_opt(
    "modules/railviz/test_resources/schedule", "20151121", 2, false, true,
    false, true);

}  // namespace railviz
}  // namespace motis
