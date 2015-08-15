#include "motis/routing/response_builder.h"

#include "boost/date_time/gregorian/gregorian_types.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

#include "motis/protocol/RoutingResponse_generated.h"

using namespace flatbuffers;
using namespace motis::module;

namespace motis {
namespace routing {

uint64_t iso_to_unix_time(std::string const& str) {
  if (str.length() != 16) {
    return 0;
  }

  // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
  // 2 0 1 5 - 0 3 - 2 1 T  1  0  :  4  3
  int year = std::stoi(str.substr(0, 4));
  int month = std::stoi(str.substr(5, 2));
  int day = std::stoi(str.substr(8, 2));
  int hours = std::stoi(str.substr(11, 2));
  int minutes = std::stoi(str.substr(14, 2));
  int total_minutes = hours * 60 + minutes;

  boost::posix_time::ptime t(boost::gregorian::date(year, month, day),
                             boost::posix_time::minutes(total_minutes));
  boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));

  return (t - epoch).total_seconds();
}

std::vector<Offset<Stop>> convert_stops(
    FlatBufferBuilder& b, std::vector<journey::stop> const& stops) {
  std::vector<Offset<Stop>> buf_stops;

  for (auto const& stop : stops) {
    auto arr = CreateEventInfo(b, iso_to_unix_time(stop.arrival.date_time),
                               b.CreateString(stop.arrival.platform));
    auto dep = CreateEventInfo(b, iso_to_unix_time(stop.departure.date_time),
                               b.CreateString(stop.departure.platform));
    buf_stops.push_back(CreateStop(b, stop.eva_no, b.CreateString(stop.name),
                                   arr, dep, stop.interchange));
  }

  return buf_stops;
}

std::vector<Offset<MoveWrapper>> convert_moves(
    FlatBufferBuilder& b, std::vector<journey::transport> const& transports) {
  std::vector<Offset<MoveWrapper>> moves;

  for (auto const& t : transports) {
    Range r(t.from, t.to);
    if (t.walk) {
      moves.push_back(
          CreateMoveWrapper(b, Move_Walk, CreateWalk(b, &r).Union()));
    } else {
      moves.push_back(CreateMoveWrapper(
          b, Move_Transport,
          CreateTransport(b, &r, b.CreateString(t.category_name), t.train_nr,
                          b.CreateString(t.line_identifier)).Union()));
    }
  }

  return moves;
}

std::vector<Offset<Attribute>> convert_attributes(
    FlatBufferBuilder& b, std::vector<journey::attribute> const& attributes) {
  std::vector<Offset<Attribute>> buf_attributes;
  for (auto const& a : attributes) {
    buf_attributes.push_back(CreateAttribute(
        b, a.from, a.to, b.CreateString(a.code), b.CreateString(a.text)));
  }
  return buf_attributes;
}

motis::module::msg_ptr journeys_to_message(
    std::vector<journey> const& journeys) {
  FlatBufferBuilder b;

  std::vector<Offset<Connection>> connections;
  for (auto const& j : journeys) {
    connections.push_back(
        CreateConnection(b, b.CreateVector(convert_stops(b, j.stops)),
                         b.CreateVector(convert_moves(b, j.transports)),
                         b.CreateVector(convert_attributes(b, j.attributes))));
  }

  b.Finish(CreateMessage(
      b, MsgContent_RoutingResponse,
      CreateRoutingResponse(b, b.CreateVector(connections)).Union()));

  return make_msg(b);
}

}  // namespace routing
}  // namespace motis
