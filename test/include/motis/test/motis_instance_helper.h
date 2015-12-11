#pragma once

#include <memory>
#include <string>
#include <vector>

#include "boost/system/system_error.hpp"

#include "conf/options_parser.h"

#include "motis/bootstrap/motis_instance.h"
#include "motis/core/common/util.h"
#include "motis/module/message.h"

namespace motis {
namespace test {

std::unique_ptr<motis::bootstrap::motis_instance> launch_motis(
    std::string const& dataset, std::string const& schedule_begin,
    std::vector<std::string> const& modules,
    std::vector<std::string> const& modules_cmdline_opt = {});

module::msg_ptr send(
    std::unique_ptr<motis::bootstrap::motis_instance> const& instance,
    module::msg_ptr request);

}  // namespace test
}  // namespace motis
