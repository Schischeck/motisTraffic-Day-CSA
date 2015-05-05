#ifndef TD_EXECUTION_DATASET_SETTINGS_H_
#define TD_EXECUTION_DATASET_SETTINGS_H_

#include <string>

#include "conf/configuration.h"

namespace boost {
namespace program_options {
class options_description;
}
}

namespace td {
namespace execution {

class dataset_settings : public conf::configuration {
public:
  dataset_settings(std::string default_dataset);

  virtual ~dataset_settings() { }

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;

  std::string dataset;
};

}  // namespace execution
}  // namespace td

#endif  // TD_EXECUTION_DATASET_SETTINGS_H_
