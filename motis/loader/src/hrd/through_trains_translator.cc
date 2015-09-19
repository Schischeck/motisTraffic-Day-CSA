#include "motis/loader/parsers/hrd/through_trains_translator.h"

#include "parser/util.h"

#include "motis/loader/util.h"

namespace motis {
namespace loader {
namespace hrd {

through_trains_translator::through_trains_translator(
    through_trains_map& through_trains)
    : through_trains_(through_trains) {}

void through_trains_translator::try_apply_rule(
    hrd_service&& s, std::vector<hrd_service>& result) {
  // TODO
  //  for (unsigned i = 2; i < s.stops_.size() - 1; ++i) {
  //    auto const& from_stop = s.stops_[i - 1];
  //    auto const& to_stop = s.stops_[i];
  //    auto const& section = s.sections_[i - 1];
  //    auto const& admin = raw_to_int<uint64_t>(section.admin);
  //
  //    auto from_stop_key =
  //        std::make_tuple(false, section.train_num, admin, from_stop.eva_num);
  //    auto to_stop_key =
  //        std::make_tuple(true, section.train_num, admin, to_stop.eva_num);
  //
  //    auto from_stop_it = through_trains_.find(from_stop_key);
  //    auto to_stop_it = through_trains_.find(to_stop_key);
  //
  //    verify(to_stop_it == end(through_trains_), "bad through train end");
  //    verify(from_stop_it == end(through_trains_), "bad through train begin");
  //  }

  result.emplace_back(std::move(s));
}

}  // hrd
}  // loader
}  // motis
