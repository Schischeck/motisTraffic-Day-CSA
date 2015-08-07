#include "motis/loader/parser.h"

namespace motis {
namespace loader {
namespace gtfs {

struct gtfs_parser : public parser {
  virtual bool applicable(boost::filesystem::path const& path) override;
  virtual void parse(boost::filesystem::path const& path) override;
};

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
