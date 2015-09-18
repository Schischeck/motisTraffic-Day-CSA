#include "motis/loader/parsers/hrd/service/through_trains_parser.h"

#include "parser/cstr.h"

#include "motis/loader/util.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace parser;

bool through_train_rules::try_apply_rule(hrd_service const& s,
                                         std::vector<hrd_service>& result) {
  return true;
}

through_train_rules parse_through_train_rules(loaded_file const& src) {
  through_train_rules rules;
  for_each_line_numbered(src.content, [&](cstr line, int line_number) {

  });

  return rules;
}

}  // hrd
}  // loader
}  // motis
