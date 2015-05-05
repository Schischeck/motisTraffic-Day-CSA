#include "execution/query_file_settings.h"

#include <ostream>

#include "boost/program_options.hpp"

#define QUERY_FILE "query_file"

namespace po = boost::program_options;

namespace td {
namespace execution {

query_file_settings::query_file_settings(std::string default_query_file)
    : query_file(default_query_file) {
}

boost::program_options::options_description query_file_settings::desc() {
  po::options_description desc("Listener Options");
  desc.add_options()
    (QUERY_FILE,
        po::value<std::string>(&query_file)->default_value(query_file),
        "path to file containing queries to process");
  return desc;
}

void query_file_settings::print(std::ostream& out) const {
  out << "  " << QUERY_FILE << ": " << query_file;
}

}  // namespace execution
}  // namespace td
