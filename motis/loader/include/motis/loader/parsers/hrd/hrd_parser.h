#include "motis/loader/parser.h"

namespace motis {
namespace loader {
namespace hrd {

struct hrd_parser : public parser {
  virtual bool applicable(std::string const& path) override;
  virtual void parse(std::string const& path) override;
};

}  // namespace hrd
}  // namespace loader
}  // namespace motis