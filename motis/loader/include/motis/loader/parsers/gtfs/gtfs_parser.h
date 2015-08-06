#include "motis/loader/parser.h"

namespace motis {
namespace loader {
namespace gtfs {

struct gtfs_parser : public parser {
  virtual bool applicable(std::string const& path) override;
  virtual void parse(std::string const& path) override;
};

}  // namespace gtfs
}  // namespace loader
}  // namespace motis