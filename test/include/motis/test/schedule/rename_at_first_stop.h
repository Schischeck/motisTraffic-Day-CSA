#pragma once

#include "motis/loader/loader_options.h"

namespace motis {
namespace test {
namespace schedule {
namespace rename_at_first_stop {

static loader::loader_options dataset_opt("test/schedule/rename_at_first_stop",
                                          "20160128");

constexpr auto kRisFolderArg =
    "--ris.input_folder=test/schedule/rename_at_first_stop/ris";

}  // namespace rename_at_first_stop
}  // namespace schedule
}  // namespace test
}  // namespace motis
