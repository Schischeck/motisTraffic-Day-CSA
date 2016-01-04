#pragma once

#include <string>

#include "boost/program_options.hpp"

#include "conf/configuration.h"

namespace motis {
namespace launcher {

class listener_settings : public conf::configuration {
public:
  listener_settings(bool listen_ws, bool listen_http, bool listen_tcp,
                    std::string ws_host, std::string ws_port,
                    std::string http_host, std::string http_port,
                    std::string tcp_host, std::string tcp_port,
                    std::string api_key);

  virtual ~listener_settings() {}

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;

  bool listen_ws, listen_http, listen_tcp;
  std::string ws_host, ws_port;
  std::string http_host, http_port;
  std::string tcp_host, tcp_port;
  std::string api_key;
};

}  // namespace launcher
}  // namespace motis
