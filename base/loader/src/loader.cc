#include "motis/loader/loader.h"

#include <fstream>
#include <istream>
#include <memory>
#include <ostream>
#include <vector>

#include "boost/filesystem.hpp"

#include "websocketpp/common/md5.hpp"

#include "flatbuffers/flatbuffers.h"

#include "parser/file.h"

#include "motis/core/common/logging.h"
#include "motis/loader/build_graph.h"
#include "motis/loader/gtfs/gtfs_parser.h"
#include "motis/loader/hrd/hrd_parser.h"

#include "motis/schedule-format/Schedule_generated.h"

namespace fs = boost::filesystem;
using namespace flatbuffers64;
using namespace parser;
using namespace motis::logging;

namespace motis {
namespace loader {

std::vector<std::unique_ptr<format_parser>> parsers() {
  std::vector<std::unique_ptr<format_parser>> p;
  p.emplace_back(new gtfs::gtfs_parser());
  p.emplace_back(new hrd::hrd_parser());
  return p;
}

schedule_ptr load_schedule(loader_options const& opt) {
  scoped_timer time("loading schedule");

  time_t from, to;
  std::tie(from, to) = opt.interval();
  auto binary_schedule_file = fs::path(opt.dataset_) / SCHEDULE_FILE;

  if (fs::is_regular_file(binary_schedule_file)) {
    auto buf = file(binary_schedule_file.string().c_str(), "r").content();
    return build_graph(GetSchedule(buf.buf_), from, to, opt.unique_check_,
                       opt.apply_rules_, opt.adjust_footpaths_);
  } else {
    for (auto const& parser : parsers()) {
      if (parser->applicable(opt.dataset_)) {
        FlatBufferBuilder builder;
        parser->parse(opt.dataset_, builder);
        if (opt.write_serialized_) {
          parser::file(binary_schedule_file.string().c_str(), "w+")
              .write(builder.GetBufferPointer(), builder.GetSize());
        }
        return build_graph(GetSchedule(builder.GetBufferPointer()), from, to,
                           opt.unique_check_, opt.apply_rules_,
                           opt.adjust_footpaths_);
      }
    }

    for (auto const& parser : parsers()) {
      std::cout << "missing files:\n";
      for (auto const& file : parser->missing_files(opt.dataset_)) {
        std::cout << "  " << file << "\n";
      }
    }
    throw std::runtime_error("no parser was applicable");
  }
}

}  // namespace loader
}  // namespace motis
