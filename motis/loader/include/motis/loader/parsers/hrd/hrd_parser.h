#include "motis/loader/parser.h"

#include "boost/filesystem.hpp"

#include "flatbuffers/flatbuffers.h"

#include "motis/loader/parsers/hrd/service/shared_data.h"
#include "motis/loader/parsers/hrd/service/service_builder.h"

namespace motis {
namespace loader {
namespace hrd {

struct hrd_parser : public format_parser {
  virtual bool applicable(boost::filesystem::path const& path) override;
  virtual std::vector<std::string> missing_files(
      boost::filesystem::path const& path) const override;
  virtual void parse(boost::filesystem::path const& path,
                     flatbuffers::FlatBufferBuilder&) override;

  shared_data parse_shared_data(boost::filesystem::path const& hrd_root,
                                flatbuffers::FlatBufferBuilder&);

  void parse_services_files(boost::filesystem::path const& hrd_root,
                            service_builder&);

  void parse_services_file(boost::filesystem::path const& services_file_path,
                           service_builder&);
};

}  // namespace hrd
}  // namespace loader
}  // namespace motis
