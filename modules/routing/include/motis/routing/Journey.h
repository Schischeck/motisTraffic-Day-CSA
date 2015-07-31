#ifndef TD_JOURNEY_H_
#define TD_JOURNEY_H_

#include <string>
#include <vector>

#include "motis/routing/Label.h"

namespace td {

class Label;
struct Schedule;

struct Journey
{
  Journey() = default;
  Journey(Label const* label, Schedule const& sched);

  struct Transport
  {
    int from, to;
    bool walk;
    std::string name;
    std::string categoryName;
    int categoryId;
    int trainNr;
    int duration;
    int slot;
  };

  struct Stop
  {
    int index;
    bool interchange;
    std::string name;
    std::string evaNo;
    double lat, lng;
    struct EventInfo {
      bool valid;
      std::string dateTime;
      std::string platform;
    } arrival, departure;
  };

  struct Attribute
  {
    int from, to;
    std::string code;
    std::string text;
  };

  std::string date;
  int duration, transfers, price;
  std::vector<Stop> stops;
  std::vector<Transport> transports;
  std::vector<Attribute> attributes;
};

}  // namespace td

#endif  // TD_JOURNEY_H_