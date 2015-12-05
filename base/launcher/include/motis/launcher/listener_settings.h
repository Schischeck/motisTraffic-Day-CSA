#pragma once

#include <string>

#include "boost/program_options.hpp"

#include "conf/configuration.h"

namespace motis {
namespace launcher {

class listener_settings : public conf::configuration {
public:
  listener_settings(std::string default_host, std::string default_port,
                    std::string api_key);

  virtual ~listener_settings() {}

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;

  std::string host, port;
  std::string api_key;
};

}  // namespace launcher
}  // namespace motis
