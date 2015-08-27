#include "motis/loader/loader.h"

#include <memory>
#include <vector>

#include "boost/filesystem.hpp"

#include "flatbuffers/flatbuffers.h"

#include "parser/file.h"
#include "motis/loader/parsers/gtfs/gtfs_parser.h"
#include "motis/loader/parsers/hrd/hrd_parser.h"

namespace fs = boost::filesystem;
using namespace flatbuffers;

namespace motis {
namespace loader {

std::vector<std::unique_ptr<format_parser>> parsers() {
  std::vector<std::unique_ptr<format_parser>> p;
  p.emplace_back(new gtfs::gtfs_parser());
  p.emplace_back(new hrd::hrd_parser());
  return p;
}

schedule_ptr build_graph(fs::path const&) { return {}; }

schedule_ptr load_schedule(std::string const& path) {
  auto binary_schedule_file = fs::path(path) / SCHEDULE_FILE;

  if (fs::is_regular_file(binary_schedule_file)) {
    return build_graph(binary_schedule_file);
  } else {
    for (auto const& parser : parsers()) {
      if (parser->applicable(path)) {
        FlatBufferBuilder builder;
        parser->parse(path, builder);
        parser::file(binary_schedule_file.string().c_str(), "w")
            .write(builder.GetBufferPointer(), builder.GetSize());
        return build_graph(binary_schedule_file);
      }
    }

    throw std::runtime_error("no parser was applicable");
  }
}

}  // namespace loader
}  // namespace motis
