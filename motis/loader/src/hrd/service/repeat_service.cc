#include "motis/loader/parsers/hrd/service/repeat_service.h"
#include "motis/loader/util.h"

namespace motis {
namespace loader {
namespace hrd {

hrd_service::event update_event(hrd_service::event const& origin, int interval,
                                int repetition) {
  auto const new_time = origin.time != hrd_service::NOT_SET
                            ? origin.time + (interval * repetition)
                            : hrd_service::NOT_SET;
  return {new_time, origin.in_out_allowed};
}

hrd_service create_repetition(hrd_service const& origin, int repetition) {
  return {
      origin.origin_, 0, 0,
      transform_to_vec(begin(origin.stops_), end(origin.stops_),
                       [&origin, &repetition](hrd_service::stop const& s) {
                         return hrd_service::stop{
                             s.eva_num,
                             update_event(s.arr, origin.interval_, repetition),
                             update_event(s.dep, origin.interval_, repetition)};
                       }),
      origin.sections_, origin.traffic_days_};
}

void expand_repetitions(std::vector<hrd_service>& services) {
  int size = services.size();
  services.reserve(services.size() * (services[0].num_repetitions_ + 1));
  for (int service_idx = 0; service_idx < size; ++service_idx) {
    auto const& service = services[service_idx];
    for (int repetition = 1; repetition <= service.num_repetitions_;
         ++repetition) {
      services.push_back(create_repetition(service, repetition));
    }
  }
}

}  // hrd
}  // loader
}  // motis
