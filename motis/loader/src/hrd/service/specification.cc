#include "motis/loader/parsers/hrd/service/specification.h"

#include <cctype>

#include "motis/loader/parser_error.h"

using namespace parser;

namespace motis {
namespace loader {
namespace hrd {

bool specification::is_empty() const {
  return internal_service.str == nullptr || internal_service.len == 0;
}

bool specification::valid() const {
  return ignore() || (!categories.empty() && stops.size() >= 2 &&
                      !traffic_days.empty() && !is_empty());
}

bool specification::ignore() const {
  return !is_empty() && !internal_service.starts_with("*Z");
}

void specification::reset() {
  internal_service = cstr(nullptr, 0);
  traffic_days.clear();
  categories.clear();
  line_information.clear();
  attributes.clear();
  stops.clear();
}

bool specification::read_line(cstr line, char const* filename,
                              int line_number) {
  if (line.len == 0) {
    return false;
  }

  if (std::isdigit(line[0])) {
    stops.push_back(line);
    return false;
  }

  if (line.len < 2 || line[0] != '*') {
    throw parser_error(filename, line_number);
  }

  // ignore *I, *GR, *SH, *T, *KW, *KWZ
  bool potential_kurswagen = false;
  switch (line[1]) {
    case 'K':
      potential_kurswagen = true;
    /* no break */
    case 'Z':
    case 'T':
      if (potential_kurswagen && line.len > 3 && line[3] == 'Z') {
        // ignore KWZ line
      } else if (is_empty()) {
        internal_service = line;
      } else {
        return true;
      }
      break;
    case 'A':
      if (line.starts_with("*A VE")) {
        traffic_days.push_back(line);
      } else {  // *A based on HRD format version 5.00.8
        attributes.push_back(line);
      }
      break;
    case 'G':
      if (!line.starts_with("*GR")) {
        categories.push_back(line);
      }
      break;
    case 'L': line_information.push_back(line); break;
    case 'R': directions.push_back(line); break;
  }

  return false;
}

}  // hrd
}  // loader
}  // motis
