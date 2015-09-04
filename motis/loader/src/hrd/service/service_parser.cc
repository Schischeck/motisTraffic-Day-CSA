#include "motis/loader/parsers/hrd/service/service_parser.h"

#include <cctype>
#include <algorithm>

#include "parser/util.h"

#include "motis/core/common/logging.h"
#include "motis/loader/util.h"
#include "motis/loader/parser_error.h"

using namespace parser;
using namespace flatbuffers;
using namespace motis::logging;

namespace motis {
namespace loader {
namespace hrd {

void parse_services(loaded_file const& file,
                    std::function<void(specification const&)> builder) {
  specification spec;
  for_each_line_numbered(file.content, [&](cstr line, int line_number) {
    bool finished = spec.read_line(line, file.name, line_number);

    if (!finished) {
      return;
    }

    if (!spec.valid()) {
      LOG(error) << "skipping bad service at " << file.name << ":"
                 << line_number;
    } else if (!spec.ignore()) {
      // Store if relevant.
      try {
        builder(spec);
      } catch (std::runtime_error const& e) {
        LOG(error) << "unable to build service at " << file.name << ":"
                   << line_number << ", skipping";
      }
    }

    // Next try! Re-read first line of next service.
    spec.reset();
    spec.read_line(line, file.name, line_number);
  });

  if (spec.valid()) {
    builder(spec);
  }
}

}  // hrd
}  // loader
}  // motis
