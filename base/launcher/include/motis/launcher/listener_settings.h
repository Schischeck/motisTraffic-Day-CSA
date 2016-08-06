#pragma once

#include <string>

#include "boost/program_options.hpp"

#include "conf/configuration.h"

namespace motis {
namespace launcher {

class listener_settings : public conf::configuration {
public:
  listener_settings(bool listen_ws, bool listen_http, bool listen_tcp,
                    std::string ws_host, std::string ws_port, bool ws_binary,
                    std::string http_host, std::string http_port,
                    std::string tcp_host, std::string tcp_port,
                    std::string api_key);

  ~listener_settings() override = default;

  boost::program_options::options_description desc() override;
  void print(std::ostream& out) const override;

  bool listen_ws_, listen_http_, listen_tcp_;
  std::string ws_host_, ws_port_;
  bool ws_binary_;
  std::string http_host_, http_port_;
  std::string tcp_host_, tcp_port_;
  std::string api_key_;
};

}  // namespace launcher
}  // namespace motis
