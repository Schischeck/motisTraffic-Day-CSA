#include "motis/reliability/reliability.h"

#include <iostream>

#include "motis/module/api.h"

using namespace json11;
using namespace motis::module;

namespace motis {
namespace reliability {

std::vector<Json> get_distribution(reliability* r, Json const& msg) {
  std::cout << "Get Distribution" << std::endl;
  return {Json::object{{"status", "success"}}};
}

reliability::reliability() : ops_{{"get-distribution", get_distribution}} {}

std::vector<Json> reliability::on_msg(Json const& msg, sid) {
  auto op = ops_.find(msg["type"].string_value());
  if (op == end(ops_)) {
    return {};
  }
  return op->second(this, msg);
}

MOTIS_MODULE_DEF_MODULE(reliability)

}  // namespace reliability
}  // namespace motis
