#ifndef TD_EXECUTION_LISTENER_SETTINGS_H_
#define TD_EXECUTION_LISTENER_SETTINGS_H_

#include <string>

#include "conf/configuration.h"

namespace td {
namespace execution {

class listener_settings : public conf::configuration {
public:
  listener_settings(std::string default_host,
                    std::string default_port);

  virtual ~listener_settings() { }

  virtual boost::program_options::options_description desc() override;

  virtual void print(std::ostream& out) const override;

  std::string host, port;
};

}  // namespace execution
}  // namespace td

#endif  // TD_EXECUTION_LISTENER_SETTINGS_H_
