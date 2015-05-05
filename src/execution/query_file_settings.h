#ifndef TD_EXECUTION_QUERY_FILE_SETTINGS_H_
#define TD_EXECUTION_QUERY_FILE_SETTINGS_H_

#include <string>

#include "conf/configuration.h"

namespace td {
namespace execution {

class query_file_settings : public conf::configuration {
public:
  query_file_settings(std::string default_query_file);
  virtual ~query_file_settings() { }
  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;
  std::string query_file;
};

}  // namespace execution
}  // namespace td

#endif  // TD_EXECUTION_QUERY_FILE_SETTINGS_H_
