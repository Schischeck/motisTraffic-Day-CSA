#ifndef CONF_OPTIONS_PARSER_H_
#define CONF_OPTIONS_PARSER_H_

#include <vector>
#include <ostream>
#include <string>

#include "boost/program_options.hpp"

#include "conf/configuration.h"

namespace conf {

class options_parser {
public:
  options_parser(std::vector<configuration*> options);

  void configure_description();
  void read_command_line_args(int argc, char* argv[]);
  void read_configuration_file();
  bool help();
  void print_help(std::ostream& out);
  bool version();
  void print_used(std::ostream& out);
  void print_unrecognized(std::ostream& out);
  bool list_options() const;
  void print_options(std::ostream& out) const;

private:
  std::string file_;
  boost::program_options::options_description desc_;
  std::vector<std::string> unrecog_;
  boost::program_options::variables_map vm_;
  std::vector<configuration*> options_;
};

}  // namespace conf

#endif  // CONF_OPTIONS_PARSER_H_