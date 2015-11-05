#include <sstream>

#include "boost/lexical_cast.hpp"
#include "boost/date_time/gregorian/gregorian_types.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"

#include "motis/realtime/message_reader.h"
#include "motis/realtime/realtime_schedule.h"

namespace motis {
namespace realtime {

message_reader::message_reader(realtime_schedule& rts) : rts_(rts) {}

std::unique_ptr<message> message_reader::read_message(std::istream& in) {
  auto c = in.peek();
  while (c == '\n' || c == '\r') {
    in.get();
    c = in.peek();
  }
  switch (c) {
    case 'D': return read_delay_message(in);
    case 'A': return read_additional_train_message(in);
    case 'C': return read_cancel_train_message(in);
    case 'R': return read_reroute_train_message(in);
    case 'S':
    case 's': return read_connection_status_decision_message(in);
    case 'B':
    case 'b': return read_connection_status_assessment_message(in);
    default: {
      std::string line;
      std::getline(in, line);
      return std::unique_ptr<message>(nullptr);
    }
  }
}

std::unique_ptr<delay_message> message_reader::read_delay_message(
    std::istream& in) {
  std::unique_ptr<delay_message> msg(new delay_message);

  char tag;
  int n_is;
  int n_forecasts;

  in >> tag >> msg->release_time_ >> msg->train_nr_ >> n_is >> n_forecasts;
  assert(tag == 'D');

  msg->forecasts_.reserve(n_forecasts);
  for (int i = 0; i < n_is + n_forecasts; i++) {
    char type;
    unsigned eva;
    std::time_t schedule_ut, delayed_ut;

    in >> type >> eva >> schedule_ut >> delayed_ut;
    if (!in) {
      break;
    }

    int station_index = eva_to_station_index(eva);
    motis::time schedule_time = to_time(schedule_ut);
    motis::time delayed_time = to_time(delayed_ut);

    if (n_is > 0) {
      msg->is_ = delayed_event(station_index, msg->train_nr_, type == 'd',
                               schedule_time, delayed_time);
      n_is--;
    } else {
      msg->forecasts_.emplace_back(station_index, msg->train_nr_, type == 'd',
                                   schedule_time, delayed_time);
    }
  }

  /*
  if (in.peek() != '\n') {
    std::cout << "\nBroken delay message: rt=" << msg->release_time_ << " tr="
  << msg->train_nr_
      << " is=" << n_is << " fc=" << n_forecasts << std::endl;
    std::string rest;
    getline(in, rest);
    std::cout << "Rest of line: <" << rest << ">" << std::endl;
  }
  */
  in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

  return msg;
}

std::unique_ptr<additional_train_message>
message_reader::read_additional_train_message(std::istream& in) {
  std::unique_ptr<additional_train_message> msg(new additional_train_message);

  char tag;
  int n_stops;

  in >> tag >> msg->release_time_ >> msg->train_nr_ >> n_stops;
  assert(tag == 'A');
  std::getline(in, msg->category_, '#');

  msg->events_.reserve(n_stops);
  for (int i = 0; i < n_stops; i++) {
    char type;
    unsigned eva;
    std::time_t schedule_ut;

    in >> type >> eva >> schedule_ut;
    if (!in) {
      break;
    }

    int station_index = eva_to_station_index(eva);
    motis::time schedule_time = to_time(schedule_ut);

    if (station_index != 0 && schedule_time != motis::INVALID_TIME) {
      msg->events_.emplace_back(station_index, msg->train_nr_, type == 'd',
                                schedule_time);
    }
  }

  return msg;
}

std::unique_ptr<cancel_train_message> message_reader::read_cancel_train_message(
    std::istream& in) {
  std::unique_ptr<cancel_train_message> msg(new cancel_train_message);

  char tag;
  int n_stops;

  in >> tag >> msg->release_time_ >> msg->train_nr_ >> n_stops;
  assert(tag == 'C');

  msg->events_.reserve(n_stops);
  for (int i = 0; i < n_stops; i++) {
    char type;
    unsigned eva;
    std::time_t schedule_ut;

    in >> type >> eva >> schedule_ut;
    if (!in) {
      break;
    }

    int station_index = eva_to_station_index(eva);
    motis::time schedule_time = to_time(schedule_ut);

    if (station_index != 0 && schedule_time != motis::INVALID_TIME) {
      msg->events_.emplace_back(station_index, msg->train_nr_, type == 'd',
                                schedule_time);
    }
  }

  return msg;
}

std::unique_ptr<reroute_train_message>
message_reader::read_reroute_train_message(std::istream& in) {
  std::unique_ptr<reroute_train_message> msg(new reroute_train_message);

  char tag;
  int n_canceled;
  int n_new;

  in >> tag >> msg->release_time_ >> msg->train_nr_ >> n_canceled >> n_new;
  assert(tag == 'R');
  std::getline(in, msg->category_, '#');

  msg->canceled_events_.reserve(n_canceled);
  msg->new_events_.reserve(n_new);
  for (int i = 0; i < n_canceled + n_new; i++) {
    char type, action;
    unsigned eva;
    std::time_t schedule_ut;

    in >> type >> eva >> schedule_ut >> action;
    if (!in) {
      break;
    }

    int station_index = eva_to_station_index(eva);
    motis::time schedule_time = to_time(schedule_ut);

    if (station_index == 0 || schedule_time == motis::INVALID_TIME) {
      continue;
    }

    if (action == 'c') {
      msg->canceled_events_.emplace_back(station_index, msg->train_nr_,
                                         type == 'd', schedule_time);
    } else if (action == 'n' || action == 'N') {
      msg->new_events_.emplace_back(station_index, msg->train_nr_, type == 'd',
                                    schedule_time);
    } else {
      // unknown action
      continue;
    }
  }

  return msg;
}

status_decision to_status_decision(char c) {
  switch (c) {
    case 'k': return status_decision::kept;
    case 'u': return status_decision::unkept;
    case 'n': return status_decision::new_connection;
    default: return status_decision::unknown;
  }
}

std::unique_ptr<connection_status_decision_message>
message_reader::read_connection_status_decision_message(std::istream& in) {
  std::unique_ptr<connection_status_decision_message> msg(
      new connection_status_decision_message);

  char tag;
  char status;
  unsigned from_eva, to_eva;
  std::time_t from_ut, to_ut;

  in >> tag >> status >> from_eva >> msg->arrival_._train_nr >> from_ut;
  assert(tag == 'S' || tag == 's');

  if (tag == 's') {
    in >> to_eva;
  } else {
    to_eva = from_eva;
  }
  in >> msg->departure_._train_nr >> to_ut >> msg->release_time_;

  msg->arrival_._station_index = eva_to_station_index(from_eva);
  msg->arrival_._schedule_time = to_time(from_ut);
  msg->departure_._station_index = eva_to_station_index(to_eva);
  msg->departure_._schedule_time = to_time(to_ut);
  msg->decision_ = to_status_decision(status);

  return msg;
}

std::unique_ptr<connection_status_assessment_message>
message_reader::read_connection_status_assessment_message(std::istream& in) {
  std::unique_ptr<connection_status_assessment_message> msg(
      new connection_status_assessment_message);

  char tag;
  unsigned from_eva, to_eva;
  std::time_t from_ut, to_ut;

  in >> tag >> msg->assessment_ >> from_eva >> msg->arrival_._train_nr >>
      from_ut;
  assert(tag == 'B' || tag == 'b');

  if (tag == 'b') {
    in >> to_eva;
  } else {
    to_eva = from_eva;
  }
  in >> msg->departure_._train_nr >> to_ut >> msg->release_time_;

  msg->arrival_._station_index = eva_to_station_index(from_eva);
  msg->arrival_._schedule_time = to_time(from_ut);
  msg->departure_._station_index = eva_to_station_index(to_eva);
  msg->departure_._schedule_time = to_time(to_ut);

  return msg;
}

int message_reader::eva_to_station_index(unsigned eva) const {
  auto it =
      rts_._schedule.eva_to_station.find(boost::lexical_cast<std::string>(eva));
  if (it == std::end(rts_._schedule.eva_to_station)) {
    return 0;
  } else {
    return it->second->index;
  }
}

motis::time message_reader::to_time(std::time_t unix_ts) {
  auto it = time_cache_.find(unix_ts);
  if (it != time_cache_.end()) {
    return it->second;
  }
  // timestamps in messages are in the local timezone, not utc
  std::tm* t = std::localtime(&unix_ts);
  if (t == nullptr) return INVALID_TIME;

  auto schedule_begin =
      boost::posix_time::from_time_t(rts_._schedule.schedule_begin_).date();
  boost::gregorian::date msg_date(t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
  motis::time mt = INVALID_TIME;
  if (msg_date >= schedule_begin) {
    mt = motis::to_time((msg_date - schedule_begin).days(),
                        t->tm_hour * 60 + t->tm_min);
  }
  if (time_cache_.size() > 10000) {
    time_cache_.clear();
  }
  time_cache_[unix_ts] = mt;
  return mt;
}

}  // namespace realtime
}  // namespace motis
