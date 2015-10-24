#include "motis/loader/parser.h"

#include "boost/filesystem.hpp"

#include "flatbuffers/flatbuffers.h"

#include "motis/loader/model/hrd/shared_data.h"
#include "motis/loader/builders/hrd/service_builder.h"
#include "motis/loader/builders/hrd/rule_service_builder.h"

namespace motis {
namespace loader {
namespace hrd {

struct hrd_parser : public format_parser {
  bool applicable(boost::filesystem::path const& path) override;

  std::vector<std::string> missing_files(
      boost::filesystem::path const& path) const override;

  void parse(boost::filesystem::path const& path,
             flatbuffers::FlatBufferBuilder&) override;

  shared_data parse_shared_data(boost::filesystem::path const& hrd_root,
                                flatbuffers::FlatBufferBuilder&);

  void parse_services_files(boost::filesystem::path const& hrd_root,
                            service_builder&, rule_service_builder&);

  void parse_services_file(boost::filesystem::path const& services_file_path,
                           service_builder&, rule_service_builder&);
};

}  // namespace hrd
}  // namespace loader
}  // namespace motis
