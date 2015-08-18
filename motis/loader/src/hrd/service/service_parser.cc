#include "motis/loader/parsers/hrd/service/service_parser.h"

#include <cctype>

#include "motis/loader/util.h"
#include "motis/loader/parser_error.h"

#include "motis/loader/parsers/hrd/service/specification.h"

using namespace parser;
using namespace flatbuffers;

namespace motis {
namespace loader {
namespace hrd {

struct train_builder {

  void create_services(specification const& s, char const* filename,
                       int line_number, FlatBufferBuilder& b,
                       std::vector<Offset<Service>>& services) {
    // CreateTrain(b, route(s, b), {}, {}, {}, traffic_days(s, b), {});
  }

  std::map<std::vector<int>, Offset<Route>> routes_;
};

void parse_services(
    loaded_file file, std::map<int, Offset<Station>> const& /* stations */,
    std::map<uint16_t, Offset<Attribute>> const& /* attributes */,
    std::map<int, Offset<String>> const& /* bitfields */, platform_rules const&,
    FlatBufferBuilder& fbb, std::vector<Offset<Service>>& services) {
  train_builder tb;
  specification spec;
  for_each_line_numbered(file.content, [&](cstr line, int line_number) {
    bool finished = spec.read_line(line, file.name, line_number);

    if (!finished) {
      return;
    }

    if (!spec.valid()) {
      throw parser_error(file.name, line_number);
    }

    // Store if relevant.
    if (!spec.ignore()) {
      tb.create_services(spec, file.name, line_number, fbb, services);
    }

    // Next try! Re-read first line of next service.
    spec.reset();
    spec.read_line(line, file.name, line_number);
  });
}

}  // hrd
}  // loader
}  // motis
