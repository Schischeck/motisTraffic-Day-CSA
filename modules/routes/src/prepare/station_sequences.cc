#include "motis/routes/prepare/station_sequences.h"

#include <iostream>
#include <map>
#include <vector>

#include "motis/core/common/get_or_create.h"
#include "motis/loader/classes.h"

#include "motis/routes/prepare/fbs/use_64bit_flatbuffers.h"

#include "motis/schedule-format/Schedule_generated.h"

namespace motis {
namespace routes {

std::vector<station_seq> load_station_sequences(
    motis::loader::Schedule const* sched) {
  std::vector<station_seq> result;

  auto const& mapping = loader::class_mapping();

  std::map<motis::loader::Route const*, station_seq> seqs;
  for (auto const& service : *sched->services()) {

    auto& seq = get_or_create(seqs, service->route(), [&]() {
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

  for (auto& s : seqs) {
    result.emplace_back(std::move(s.second));
  }

  return result;
}

}  // namespace routes
}  // namespace motis
