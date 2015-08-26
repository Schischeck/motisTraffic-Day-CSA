#include "motis/loader/parser.h"

namespace motis {
namespace loader {
namespace gtfs {

struct gtfs_parser : public format_parser {
  virtual bool applicable(boost::filesystem::path const& path) override;
  virtual void parse(boost::filesystem::path const& path,
                     flatbuffers::FlatBufferBuilder&) override;
};

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
