#include "motis/path/prepare/schedule/station_sequences.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <random>
#include <vector>

#include "utl/concat.h"
#include "utl/equal_ranges.h"
#include "utl/erase_duplicates.h"
#include "utl/get_or_create.h"
#include "utl/to_vec.h"

#include "motis/core/common/logging.h"

#include "motis/loader/classes.h"

#include "motis/path/prepare/fbs/use_64bit_flatbuffers.h"

#include "motis/schedule-format/Schedule_generated.h"

using namespace motis::logging;

namespace motis {
namespace path {

std::vector<station_seq> load_station_sequences(
    motis::loader::Schedule const* sched) {
  scoped_timer timer("loading station sequences");

  auto const& mapping = loader::class_mapping();

  std::map<motis::loader::Route const*, station_seq> seqs;
  for (auto const& service : *sched->services()) {

    auto& seq = utl::get_or_create(seqs, service->route(), [&] {
      station_seq seq;
      for (auto const& station : *service->route()->stations()) {
        seq.station_ids_.emplace_back(station->id()->str());
        seq.coordinates_.emplace_back(station->lat(), station->lng());
      }
      return seq;
    });

    for (auto const& section : *service->sections()) {
      seq.train_nrs_.insert(section->train_nr());

      auto it = mapping.find(section->category()->name()->str());
      if (it != end(mapping)) {
        seq.categories_.emplace(it->second);
      }
    }
  }

  auto sequences =
      utl::to_vec(seqs, [](auto const& pair) { return pair.second; });

  std::vector<station_seq> result;
  utl::equal_ranges(
      sequences,
      [](auto const& lhs, auto const& rhs) {
        return lhs.station_ids_ < rhs.station_ids_;
      },
      [&](auto const& lb, auto const& ub) {
        auto elem = *lb;

        for (auto it = std::next(lb); it != ub; ++it) {
          elem.categories_.insert(begin(it->categories_), end(it->categories_));
          elem.train_nrs_.insert(begin(it->train_nrs_), end(it->train_nrs_));
        }

        result.emplace_back(elem);
      });

  std::mt19937 g(0);
  std::shuffle(begin(result), end(result), g);

  LOG(motis::logging::info) << result.size() << " station sequences "
                            << "(was: " << sequences.size() << ")";

  return result;
}

}  // namespace path
}  // namespace motis
