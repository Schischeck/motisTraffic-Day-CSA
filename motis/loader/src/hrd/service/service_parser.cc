#include "motis/loader/parsers/hrd/service_parser.h"

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

void parse_specification(loaded_file const& file,
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

  if (!spec.is_empty() && spec.valid() && !spec.ignore()) {
    builder(spec);
  }
}

void expand_and_consume(hrd_service&& non_expanded_service,
                        std::map<int, bitfield> const& bitfields,
                        std::function<void(hrd_service const&)> consumer) {
  std::vector<hrd_service> expanded_services;
  expand_traffic_days(non_expanded_service, bitfields, expanded_services);
  expand_repetitions(expanded_services);
  std::for_each(begin(expanded_services), end(expanded_services), consumer);
}

void for_each_service(loaded_file const& file,
                      std::map<int, bitfield> const& bitfields,
                      std::function<void(hrd_service const&)> consumer) {
  parse_specification(file, [&](specification const& spec) {
    try {
      expand_and_consume(hrd_service(spec), bitfields, consumer);
    } catch (parser_error const& e) {
      LOG(error) << "skipping bad service at " << e.filename << ":"
                 << e.line_number;
    }
  });
}

}  // hrd
}  // loader
}  // motis
