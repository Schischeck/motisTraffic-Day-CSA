#include "motis/loader/parser.h"

namespace motis {
namespace loader {
namespace hrd {

struct hrd_parser : public format_parser {
  virtual bool applicable(boost::filesystem::path const& path) override;
  virtual void parse(boost::filesystem::path const& path) override;
};

}  // namespace hrd
}  // namespace loader
}  // namespace motis
