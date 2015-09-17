#include "motis/loader/loader.h"

#include <memory>
#include <vector>
#include <fstream>
#include <istream>
#include <ostream>

#include "boost/filesystem.hpp"

#include "websocketpp/common/md5.hpp"

#include "flatbuffers/flatbuffers.h"

#include "parser/file.h"

#include "motis/core/common/logging.h"
#include "motis/loader/graph_builder.h"
#include "motis/loader/parsers/gtfs/gtfs_parser.h"
#include "motis/loader/parsers/hrd/hrd_parser.h"

namespace fs = boost::filesystem;
using namespace flatbuffers;
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

schedule_ptr load_schedule(std::string const& path, time_t from, time_t to) {
  scoped_timer time("loading schedule");

  auto binary_schedule_file = fs::path(path) / SCHEDULE_FILE;

  if (fs::is_regular_file(binary_schedule_file)) {
    LOG(info) << "loading schedule file " << binary_schedule_file.string().c_str();

	std::size_t size = file(binary_schedule_file.string().c_str(), "r").size();
	std::ifstream input(binary_schedule_file.string().c_str(), std::ios_base::binary);
	input.exceptions(std::ios_base::failbit | std::ios_base::badbit);
	std::vector<unsigned char> bytes(size);
	input.read(reinterpret_cast<char*>(&bytes[0]), bytes.size());
	std::cout << "\n\nMD5: " << websocketpp::md5::md5_hash_hex(&bytes[0], bytes.size()) << "\n\n";

	input.close();

    return build_graph(GetSchedule(&bytes[0]), from, to);
  } else {
    for (auto const& parser : parsers()) {
      if (parser->applicable(path)) {
        FlatBufferBuilder builder;
		parser->parse(path, builder);

		std::cout << "\n\nMD5: " << websocketpp::md5::md5_hash_hex(builder.GetBufferPointer(), builder.GetSize()) << "\n\n";
		std::ofstream output(binary_schedule_file.string().c_str(), std::ios_base::binary);
		output.write(reinterpret_cast<char*>(builder.GetBufferPointer()), builder.GetSize());
		output.close();

        return build_graph(GetSchedule(builder.GetBufferPointer()), from, to);
      }
    }

    for (auto const& parser : parsers()) {
      std::cout << "missing files:\n";
      for (auto const& file : parser->missing_files(path)) {
        std::cout << "  " << file << "\n";
      }
    }
    throw std::runtime_error("no parser was applicable");
  }
}

}  // namespace loader
}  // namespace motis
