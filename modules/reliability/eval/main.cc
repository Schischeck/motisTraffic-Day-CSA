#include <array>
#include <fstream>
#include <iostream>
#include <stdlib.h>

#include "motis/core/common/date_time_util.h"
#include "motis/core/schedule/schedule.h"
#include "motis/loader/loader.h"

#include "motis/reliability/reliability.h"
#include "motis/reliability/tools/flatbuffers_tools.h"
#include "motis/reliability/tools/system.h"

using namespace motis::reliability;

unsigned int file_index = 0;
void cb(motis::module::msg_ptr msg, boost::system::error_code e) {
  std::stringstream sst;
  if (e) {
    std::ofstream os(sst.str());
    os << "ERROR" << std::endl;
    return;
  }

  std::string prefix = "";
  auto response = msg->content<ReliableRoutingResponse const*>();

  if (response->connection_graphs()->size() > 0) {
    prefix = "reliable";
    for (auto it = response->connection_graphs()->begin();
         it != response->connection_graphs()->end(); ++it) {
      if (it->arrival_distribution()->sum() < 0.8) {
        prefix = "unreliable";
        break;
      }
    }
    sst << "results/" << prefix << "_" << file_index++ << ".json";
    std::ofstream os(sst.str());
    os << msg->to_json() << std::flush;
  }
}

uint64_t events_count(std::array<uint64_t, 10> const& events) {
  uint64_t sum = 0;
  for (auto const e : events) {
    sum += e;
  }
  return sum;
}

motis::station const& get_station(
    std::vector<motis::station_ptr> const& stations) {
  unsigned int idx = rand() % stations.size();
  while (events_count(stations.at(idx)->dep_class_events) < 500) {
    idx = rand() % stations.size();
  }
  return *stations.at(idx);
}

int main(int, char**) {
  auto const schedule = motis::loader::load_schedule(
      "/data/schedule/rohdaten/", true, motis::to_unix_time(2015, 10, 19),
      motis::to_unix_time(2015, 10, 20));
  auto const date = std::make_tuple(19, 10, 2015);
  system_tools::setup setup(schedule.get(), 8, false);

  srand(time(NULL));

  std::ofstream os_queries("results/os_queries.txt");

  for (unsigned int i = 0; i < 1000; ++i) {
    auto const departure_station = get_station(schedule->stations);
    auto const arrival_station = get_station(schedule->stations);
    motis::time departure_time = rand() % 1440;

    auto msg = flatbuffers_tools::to_reliable_routing_request(
        departure_station.name, departure_station.eva_nr, arrival_station.name,
        arrival_station.eva_nr, departure_time, departure_time + 120, date, 1);

    os_queries << "\n\nQuery " << i << ": " << departure_station.name << " "
               << departure_station.eva_nr << " " << arrival_station.name << " "
               << arrival_station.eva_nr << " " << departure_time << " "
               << departure_time + 120 << " " << std::get<0>(date) << "-"
               << std::get<1>(date) << "-" << std::get<2>(date) << std::endl;
    os_queries << msg->to_json() << std::endl;

    setup.dispatcher_.on_msg(msg, 0, cb);
  }

  setup.ios_.run();

  return 0;
}
