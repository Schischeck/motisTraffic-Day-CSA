#include "motis/loader/parsers/hrd/trains_parser.h"

#include <cctype>

#include "motis/loader/util.h"
#include "motis/loader/parser_error.h"

using namespace parser;
using namespace flatbuffers;

namespace motis {
namespace loader {
namespace hrd {

struct service {
  service() : internal_service(nullptr, 0) {}

  bool is_empty() const {
    return internal_service.str == nullptr || internal_service.len == 0;
  }

  bool valid() const {
    return ignore() || (!categories.empty() && stops.size() >= 2 &&
                        !travel_days.empty() && !is_empty());
  }

  bool ignore() const {
    return !is_empty() && !internal_service.starts_with("*Z");
  }

  void reset() {
    internal_service = cstr(nullptr, 0);
    travel_days.clear();
    categories.clear();
    line_information.clear();
    attributes.clear();
    stops.clear();
  }

  cstr internal_service;
  std::vector<cstr> travel_days;
  std::vector<cstr> categories;
  std::vector<cstr> line_information;
  std::vector<cstr> attributes;
  std::vector<cstr> stops;
};

bool read_line(cstr line, char const* filename, int line_number,
               service& current_service) {
  if (line.len == 0) {
    return false;
  }

  if (std::isdigit(line[0])) {
    current_service.stops.push_back(line);
    return false;
  }

  if (line.len < 2 || line[0] != '*') {
    throw parser_error(filename, line_number);
  }

  // ignore *I, *R, *GR, *SH, *T, *KW, *KWZ
  bool potential_kurswagen = false;
  switch (line[1]) {
    case 'K':
      potential_kurswagen = true;
    case 'Z':
    case 'T':
      if (potential_kurswagen && line.len > 3 && line[3] == 'Z') {
        // ignore KWZ line
      } else if (current_service.is_empty()) {
        current_service.internal_service = line;
      } else {
        return true;
      }
      break;
    case 'A':
      if (line.starts_with("*A VE")) {
        current_service.travel_days.push_back(line);
      } else {  // *A based on HRD format version 5.00.8
        current_service.attributes.push_back(line);
      }
      break;
    case 'G':
      if (!line.starts_with("*GR")) {
        current_service.categories.push_back(line);
      }
      break;
    case 'L':
      current_service.line_information.push_back(line);
      break;
  }

  return false;
}

Offset<Train> create_train(service const& /* s */, char const* /* filename */,
                           int /* line_number */, FlatBufferBuilder& b) {
  return CreateTrain(b, {}, {}, {}, {}, {}, {});
}

void parse_trains(loaded_file file,
                  std::map<int, Offset<Station>> const& /* stations */,
                  std::map<uint16_t, Offset<Attribute>> const& /* attributes */,
                  std::map<int, Offset<String>> const& /* bitfields */,
                  platform_rules const&, FlatBufferBuilder& b,
                  std::vector<Offset<Train>>& trains) {
  service current_service;
  for_each_line_numbered(file.content, [&](cstr line, int line_number) {
    bool finished = read_line(line, file.name, line_number, current_service);

    if (!finished) {
      return;
    }

    if (!current_service.valid()) {
      throw parser_error(file.name, line_number);
    }

    // Store if relevant.
    if (!current_service.ignore()) {
      trains.push_back(
          create_train(current_service, file.name, line_number, b));
    }

    // Next try! Re-read first line of next service.
    current_service.reset();
    read_line(line, file.name, line_number, current_service);
  });
}

}  // hrd
}  // loader
}  // motis
