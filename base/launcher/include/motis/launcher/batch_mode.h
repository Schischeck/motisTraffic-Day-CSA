#pragma once

#include <string>

#include "boost/asio/io_service.hpp"

#include "motis/module/receiver.h"

namespace motis {
namespace launcher {

void inject_queries(boost::asio::io_service&, motis::module::receiver&,
                    std::string const& input_file_path,
                    std::string const& output_file_path);

}  // namespace launcher
}  // namespace motis
