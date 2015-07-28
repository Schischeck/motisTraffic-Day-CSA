#include "motis/realtime/message_classes.h"
#include "motis/realtime/message_handler.h"
#include "motis/core/schedule/station.h"

namespace motis {
namespace realtime {

using namespace std;

message_class::parse_result message_class::_parse_result = message_class::OK;

void message_class::set_parse_result(parse_result result) {
  _parse_result = result;
}

bool message_class::parse_error_occured() {
  return _parse_result != message_class::OK;
}

void message_class::reset_parse_result() { _parse_result = message_class::OK; }

/// use int to transport category (debug only - string transport required to
/// avoid parsing category in DG)
//#define CATEGORY_AS_INT_DEBUG

/**
 * convert given message_stop::type to string (departure, arrival, begin, or
 * end)
 */
std::string stop_type_to_string(message_stop::type type) {
  switch (type) {
    case message_stop::departure:
      return "departure";
    case message_stop::arrival:
      return "arrival";
    case message_stop::first_stop:
      return "begin";
    case message_stop::last_stop:
      return "end";
  }
  return "unknown";
}

message_stop::type type_char_to_stop_type(char type) {
  switch (type) {
    case 'd':
      return message_stop::departure;
    case 'a':
      return message_stop::arrival;
    case 'b':
      return message_stop::first_stop;
    case 'e':
      return message_stop::last_stop;
  }
  return message_stop::last_stop;  // unknown type
}

std::string timestamp_reason_to_string(
    message_stop::timestamp_reason_type const& reason) {
  switch (reason) {
    case message_stop::schedule:
      return "schedule";
    case message_stop::is_message:
      return "is_message";
    case message_stop::forecast:
      return "forecast";
    case message_stop::propagation:
      return "propagation";
  }
  return "unknown";
}

/******************************************************************************/
// methods of message_class
/******************************************************************************/

/**
 * output message to stream (serialize)
 */
void message_class::write_to_stream(std::ostream& output_stream) const {
  (void)output_stream;
  cerr << "write to stream not supported for this message type: "
       << to_string(get_type()) << endl;
}

message_class* message_class::read_message_from_stream(
    std::istream& input_stream, bool eva_numbers,
    message_handler const* message_handler) {
  char c;
  input_stream >> c;
  message_class::reset_parse_result();
  //  cout << "message_type: " << c << endl;
  switch (char_to_type(c)) {
    case message_class_type::delayed_train_message:
      // cout << "delay" << endl;
      return new delayed_train_message(input_stream, eva_numbers,
                                       message_handler);
      break;
    case message_class_type::additional_train_message:
      // cout << "additional" << endl;
      return new additional_or_canceled_train_message(
          input_stream, false, eva_numbers, message_handler);
      break;
    case message_class_type::canceled_train_message:
      // cout << "canceled" << endl;
      return new additional_or_canceled_train_message(
          input_stream, true, eva_numbers, message_handler);
      break;
    case message_class_type::connection_status_decision_message:
      return new connection_status_decision_message(
          input_stream, false, eva_numbers, message_handler);
      break;
    case message_class_type::
        connection_status_decision_message_different_stations:
      return new connection_status_decision_message(
          input_stream, true, eva_numbers, message_handler);
      break;
    case message_class_type::connection_status_assessment_db_message:
      return new connection_status_assessment_db_message(
          input_stream, false, eva_numbers, message_handler);
      break;
    case message_class_type::
        connection_status_assessment_db_message_different_stations:
      return new connection_status_assessment_db_message(
          input_stream, true, eva_numbers, message_handler);
      break;
    case message_class_type::reroute_train_message:
      return new reroute_train_message(input_stream, eva_numbers,
                                       message_handler);
      break;
    case message_class_type::reroute_train_message_sg:
      return new reroute_train_message_sg(input_stream, eva_numbers,
                                          message_handler);
      break;
    case message_class_type::no_message:
      break;
  }
  return nullptr;
}

/******************************************************************************/
// methods of delayed_train_message
/******************************************************************************/

delayed_train_message::delayed_train_message(
    std::istream& stream, bool eva_numbers,
    message_handler const* message_handler)
    : message_class(message_class_type::delayed_train_message) {

  char type_char;
  message_stop::type type;
  long seconds_scheduled_time;
  long seconds_new_time;
  long seconds;
  unsigned int array_size;
  bool delay_object;
  int station_index;

  stream >> seconds >> train_index >> delay_object >> array_size;

  forecasts.resize(array_size);
  for (unsigned int i = 0; i < array_size; i++) {
    forecasts[i] = nullptr;
  }
  delay = nullptr;

  this->release_time = seconds;

  if (delay_object) {
    stream >> type_char;
    if (type_char == 'a') {
      // arrival event
      type = message_stop::arrival;
    } else {
      // departure event
      // assert (type_char == 'd');
      if (type_char != 'd') {
        cout << "\n_warning(delayed_train_message): unknown event-type: "
             << type_char << endl;

        set_parse_result(message_class::UNKNOWN_STOP_TYPE);
        return;
      }
      type = message_stop::departure;
    }

    if (eva_numbers) {
      std::string eva_number;
      stream >> eva_number;
      const station* station = message_handler->get_station_by_eva(eva_number);
      if (station == nullptr) {
        station_index = 0;
        set_parse_result(message_class::EVA_NO_CONVERSION_FAILED);
      } else {
        station_index = station->index;
      }
    } else {
      stream >> station_index;
    }

    stream >> seconds_scheduled_time >> seconds_new_time;

    // cout << "delay: " << type_char << " " << station_index << " "
    //     << seconds_scheduled_time << " " << seconds_new_time << endl;

    delay = new delayed_stop(type, station_index,
                             message_time(seconds_scheduled_time),
                             message_time(seconds_new_time));
  } else
    delay = nullptr;

  for (unsigned int i = 0; i < array_size; i++) {
    stream >> type_char;

    if (eva_numbers) {
      std::string eva_number;
      stream >> eva_number;
      const station* station = message_handler->get_station_by_eva(eva_number);
      if (station == nullptr) {
        station_index = 0;
        set_parse_result(message_class::EVA_NO_CONVERSION_FAILED);
      } else {
        station_index = station->index;
      }
    } else {
      stream >> station_index;
    }

    stream >> seconds_scheduled_time >> seconds_new_time;

    if (type_char == 'a') {
      // arrival event
      type = message_stop::arrival;
    } else {
      // departure event
      if (type_char != 'd') {
        cout << "\n_warning(delayed_train_message): unknown event-type: "
             << type_char << endl;
        set_parse_result(message_class::UNKNOWN_STOP_TYPE);
        return;
      }
      type = message_stop::departure;
    }

    // cout << "forecast[" << i << "]: " << type_char << " " << station_index
    //     << " " << seconds_scheduled_time << " " << seconds_new_time << endl;

    this->set_forecast(i, type, station_index,
                       message_time(seconds_scheduled_time),
                       message_time(seconds_new_time));

    // cout << "this->forecast: "
    //     << stop_type_to_string(this->forecasts[i]->stop_type) << " "
    //     << this->forecasts[i]->station_index << " "
    //     << this->forecasts[i]->scheduled_event_time.output() << endl;
  }
}

/**
 * output message to stream (serialize to ascii)
 */
void delayed_train_message::write_to_stream(std::ostream& output_stream) const {

  output_stream << type_to_char(type) << release_time.get_timestamp() << " "
                << train_index << " " << (delay == nullptr ? "0 " : "1 ")
                << forecasts.size();

  if (delay != nullptr)
    output_stream << ((delay->stop_type == message_stop::arrival ||
                       delay->stop_type == message_stop::last_stop)
                          ? 'a'
                          : 'd') << delay->station_index << " "
                  << delay->scheduled_event_time.get_timestamp() << " "
                  << delay->new_event_time.get_timestamp();

  for (unsigned int i = 0; i < forecasts.size(); i++) {
    output_stream << ((forecasts[i]->stop_type == message_stop::arrival ||
                       forecasts[i]->stop_type == message_stop::last_stop)
                          ? 'a'
                          : 'd') << forecasts[i]->station_index << " "
                  << forecasts[i]->scheduled_event_time.get_timestamp() << " "
                  << forecasts[i]->new_event_time.get_timestamp();
  }
  output_stream << "\n";
}

void delayed_train_message::debug_output(ostream& output_stream) const {

  output_stream << "message_type=" << type_to_char(type)
                << ", release_time=" << release_time.output()
                << ", tr_idx=" << train_index
                << ", delay_object: " << (delay == nullptr ? "no" : "yes")
                << ", num_forecasts: " << this->forecasts.size() << endl;

  if (delay != nullptr)
    output_stream << "delay: " << stop_type_to_string(delay->stop_type)
                  << ", st=" << delay->station_index
                  << ", old_time=" << delay->scheduled_event_time.output()
                  << ", new_time=" << delay->new_event_time.output() << endl;

  for (unsigned int i = 0; i < forecasts.size(); i++) {
    output_stream << "forecast[" << i
                  << "]: " << stop_type_to_string(forecasts[i]->stop_type)
                  << ", st=" << forecasts[i]->station_index << ", old_time="
                  << forecasts[i]->scheduled_event_time.output()
                  << ", new_time=" << forecasts[i]->new_event_time.output()
                  << endl;
  }
  output_stream << "\n";
}

/******************************************************************************/
// methods of additional_or_canceled_train_message
/******************************************************************************/

/**
  * constructor
  * @param type additional train or canceled train
  * @param number_of_stops number of additional or canceled stops
  * @param train index of the train
  * @param release release time of message(s)
  */
additional_or_canceled_train_message::additional_or_canceled_train_message(
    train_message_type type, unsigned int const& number_of_stops, int train,
    message_time const& release)
    : message_class((type == additional_train)
                        ? message_class_type::additional_train_message
                        : message_class_type::canceled_train_message,
                    release),
      message_type(type),
      train_index(train),
      stops(number_of_stops) {
  for (unsigned int i = 0; i < number_of_stops; i++) {
    stops[i] = nullptr;
    // cout << "messageclass constructor" << "stops[" << i << "] = " << stops[i]
    // << endl;
  }
}

/**
 * constructor for reading from stream
 */
additional_or_canceled_train_message::additional_or_canceled_train_message(
    istream& stream, bool is_canceled_train, bool eva_numbers,
    message_handler const* message_handler)
    : message_class((is_canceled_train)
                        ? message_class_type::canceled_train_message
                        : message_class_type::additional_train_message) {

  unsigned int array_size;
  long seconds;

  this->message_type = (is_canceled_train ? canceled_train : additional_train);

  stream >> seconds >> train_index >> array_size;

  if (!is_canceled_train) {
#ifndef CATEGORY_AS_INT_DEBUG
    std::string cat;
    getline(stream, cat, '#');
    set_category(cat);
#else
    int cat;
    stream >> cat;
    set_category("DZ");
#endif
  }

  release_time = message_time(seconds);

  // for stops:
  char type_char;
  message_stop::type type;
  int station_index;

  stops.resize(array_size);
  for (unsigned int i = 0; i < array_size; i++) {
    stops[i] = nullptr;
  }

  // cout << "sec " << seconds << " ti " << train_index << " as " << array_size
  // <<
  // endl;

  for (unsigned int i = 0; i < array_size; i++) {
    // stream >> type_char >> station_index >> seconds;

    stream >> type_char;

    if (eva_numbers) {
      std::string eva_number;
      stream >> eva_number;
      const station* station = message_handler->get_station_by_eva(eva_number);
      if (station == nullptr) {
        station_index = 0;
        set_parse_result(message_class::EVA_NO_CONVERSION_FAILED);
      } else {
        station_index = station->index;
      }
    } else {
      stream >> station_index;
    }

    stream >> seconds;

    if (type_char == 'a') {
      // arrival event
      type = message_stop::arrival;
    } else {
      // departure event
      // assert (type_char == 'd');
      if (type_char != 'd') {
        cout << "\n_warning(additional_or_canceled_train_message): unknown "
                "event-type: " << type_char << endl;

        set_parse_result(message_class::UNKNOWN_STOP_TYPE);
        return;
      }
      type = message_stop::departure;
    }

    stops[i] = new schedule_stop(type, station_index, message_time(seconds));
  }
}

additional_or_canceled_train_message::~additional_or_canceled_train_message() {
  for (unsigned int i = 0; i < stops.size(); i++) {
    // cout << "deleting stop " << i << " at: " << &stops[i] << endl;
    delete stops[i];
  }
}

void additional_or_canceled_train_message::write_to_stream(
    std::ostream& output_stream) const {
  output_stream << type_to_char(type) << release_time.get_timestamp() << " "
                << train_index << " " << stops.size();

  if (type == message_class_type::additional_train_message) {
    // cout << " cat: '" << get_category() << "'";
    output_stream << get_category() << '#';
  }

  for (unsigned int i = 0; i < stops.size(); i++) {
    output_stream << ((stops[i]->stop_type == message_stop::arrival ||
                       stops[i]->stop_type == message_stop::last_stop)
                          ? 'a'
                          : 'd') << stops[i]->station_index << " "
                  << stops[i]->scheduled_event_time.get_timestamp();
  }
  output_stream << "\n";
}

void additional_or_canceled_train_message::set_stop(
    unsigned int const& index, message_stop::type const& type,
    int const& station, event_time const& scheduled) {

  if (index >= stops.size()) stops.resize(index + 1);

  stops[index] = new schedule_stop(type, station, scheduled);
  // cout << "initializing stop " << index << " at: " << &stops[index] << endl;
}

void additional_or_canceled_train_message::set_stop(unsigned int const& index,
                                                    schedule_stop* stop) {
  if (index >= stops.size()) stops.resize(index + 1);
  stops[index] = stop;
}

bool additional_or_canceled_train_message::is_equal(
    const additional_or_canceled_train_message* other) const {

  if (this->get_type() == other->get_type() &&
      this->release_time == other->release_time &&
      this->category == other->category &&
      this->train_index == other->train_index &&
      this->stops.size() ==
          other->stops.size())  // TODO: sollte man alle stops überprüfen?
    return true;

  return false;
}

bool additional_or_canceled_train_message::events_are_inconsistent() const {
  // if(this->stops.size() % 2 != 0) {
  //  return true;
  //}
  for (unsigned int i = 0; i + 1 < this->stops.size(); i += 2)
    if ((this->stops[i]->stop_type != message_stop::departure &&
         this->stops[i]->stop_type != message_stop::first_stop) ||
        (this->stops[i + 1]->stop_type != message_stop::arrival &&
         this->stops[i + 1]->stop_type != message_stop::last_stop))
      return true;
  return false;
}

void additional_or_canceled_train_message::debug_output(
    ostream& output_stream) const {

  output_stream << "message_type=" << type_to_char(type)
                << ", release_time=" << release_time.output()
                << ", tr_idx=" << train_index
                << ", num_stops: " << this->stops.size()
                << ", category: " << category << endl;

  schedule_stop* stop = nullptr;
  std::string reroute_type = "?";

  output_stream << "\nstops:" << endl;

  for (unsigned int i = 0; i < stops.size(); i++) {
    stop = stops[i];

    output_stream << stop_type_to_string(stop->stop_type)
                  << ", st=" << stop->station_index
                  << ", t=" << stop->scheduled_event_time.output() << endl;
  }
  output_stream << "\n";
}

/******************************************************************************/
// methods of interchange_info_element
/******************************************************************************/

/**
 * initialization reading from stream
 */
void interchange_info_element::init_from_stream(
    istream& input_stream, bool different_stations, bool eva_numbers,
    message_handler const* message_handler) {

  long seconds_from, seconds_to;

  if (eva_numbers) {
    std::string eva_number;
    input_stream >> eva_number;
    const station* station = message_handler->get_station_by_eva(eva_number);
    if (station == nullptr) {
      station_from_index = 0;
      message_class::set_parse_result(message_class::EVA_NO_CONVERSION_FAILED);
    } else {
      station_from_index = station->index;
    }
  } else {
    input_stream >> station_from_index;
  }

  input_stream >> train_from_index >> seconds_from;

  if (different_stations) {
    if (eva_numbers) {
      std::string eva_number;
      input_stream >> eva_number;
      const station* station = message_handler->get_station_by_eva(eva_number);
      if (station == nullptr) {
        station_to_index = 0;
        message_class::set_parse_result(
            message_class::EVA_NO_CONVERSION_FAILED);
      } else {
        station_to_index = station->index;
      }
    } else {
      input_stream >> station_to_index;
    }
  } else {
    station_to_index = station_from_index;
  }

  input_stream >> train_to_index >> seconds_to;

  train_from_time = seconds_from;
  train_to_time = seconds_to;
}

void interchange_info_element::write_to_stream(std::ostream& output_stream,
                                               bool different_stations) const {
  output_stream << station_from_index << " " << train_from_index << " "
                << train_from_time.get_timestamp() << " ";
  if (different_stations) output_stream << station_to_index << " ";
  output_stream << train_to_index << " " << train_to_time.get_timestamp()
                << " ";
}

/******************************************************************************/
// methods of connection_status_info
/******************************************************************************/

/**
 * initialization reading from stream
 */
void connection_status_info::init_from_stream(
    istream& input_stream, bool different_stations, bool eva_numbers,
    message_handler const* message_handler) {
  read_and_parse_additional_info(input_stream);
  interchange_info.init_from_stream(input_stream, different_stations,
                                    eva_numbers, message_handler);

  long seconds_release;
  input_stream >> seconds_release;
  release_time = seconds_release;

  // this->write_to_stream(cout);
}

void connection_status_info::write_to_stream(
    std::ostream& output_stream) const {
  output_stream << type_to_char(type) << write_additional_info() << " ";
  interchange_info.write_to_stream(
      output_stream,
      (type == message_class_type::
                   connection_status_decision_message_different_stations) ||
          (type ==
           message_class_type::
               connection_status_assessment_db_message_different_stations));
  output_stream << release_time.get_timestamp() << "\n";
}

void connection_status_info::debug_output(ostream& output_stream) const {
  output_stream << "message_type=" << type_to_char(type)
                << ", release_time=" << release_time.output()
                << "\nstations: from "
                << this->interchange_info.station_from_index << " to "
                << this->interchange_info.station_to_index << "\ntrains: from "
                << this->interchange_info.train_from_index << " ("
                << this->interchange_info.train_from_time.output() << ")"
                << " to " << this->interchange_info.train_to_index << " ("
                << this->interchange_info.train_to_time.output() << ")"
                << "\n_status/assessment: " << this->write_additional_info()
                << endl;
}

bool connection_status_info::describes_same_interchange(
    connection_status_info* other) const {
  return interchange_info.describes_same_interchange(other->interchange_info);
}

bool interchange_info_element::describes_same_interchange(
    interchange_info_element const& other) const {
  if (this->station_from_index == other.station_from_index &&
      this->station_to_index == other.station_to_index &&
      this->train_from_index == other.train_from_index &&
      this->train_to_index == other.train_to_index &&
      this->train_from_time == other.train_from_time &&
      this->train_to_time == other.train_to_time)
    return true;
  return false;
}

/******************************************************************************/
// methods of connection_status_decision_message
/******************************************************************************/

char connection_status_to_char(connection_status::decision_type const& type) {
  switch (type) {
    case connection_status::kept:
      return 'k';
    case connection_status::unkept:
      return 'u';
    case connection_status::new_connection:
      return 'n';
    case connection_status::undecided:
      return 'd';
  }
  return 'd';
}

connection_status::decision_type char_to_connection_status(char& c) {
  switch (c) {
    case 'k':
      return connection_status::kept;
    case 'u':
      return connection_status::unkept;
    case 'n':
      return connection_status::new_connection;
    case 'd':
      return connection_status::undecided;
  }
  return connection_status::undecided;
}

std::string connection_status_decision_message::get_status_string() const {
  switch (this->status) {
    case connection_status::kept:
      return "kept";
    case connection_status::unkept:
      return "unkept";
    case connection_status::new_connection:
      return "new_connection";
    case connection_status::undecided:
      return "undecided";
  }
  return "unknown";
}

void connection_status_decision_message::read_and_parse_additional_info(
    std::istream& input_stream) {
  char status_char;
  input_stream >> status_char;
  status = char_to_connection_status(status_char);
}

std::string connection_status_decision_message::write_additional_info() const {
  stringstream ss;
  ss << connection_status_to_char(status);
  return ss.str();
}

/******************************************************************************/
// methods of reroute_train_message
/******************************************************************************/

void connection_status_assessment_db_message::read_and_parse_additional_info(
    std::istream& input_stream) {
  input_stream >> assessment;
}

std::string connection_status_assessment_db_message::write_additional_info()
    const {
  stringstream ss;
  ss << assessment;
  return ss.str();
}

/******************************************************************************/
// methods of reroute_train_message
/******************************************************************************/

/**
 * constructor
 * @param number_of_canceled_stops number of canceled stops
 * @param number_of_new_stops number of new stops
 * @param train index of the train
 * @param release release time of message(s)
 */
reroute_train_message::reroute_train_message(
    unsigned int const& number_of_canceled_stops,
    unsigned int const& number_of_new_stops, int train,
    std::string const& category, message_time const& release)
    : message_class(message_class_type::reroute_train_message, release),
      canceled_stops(number_of_canceled_stops),
      new_stops(number_of_new_stops),
      train_index(train),
      category(category) {

  for (unsigned int i = 0; i < number_of_canceled_stops; i++)
    canceled_stops[i] = nullptr;
  for (unsigned int i = 0; i < number_of_new_stops; i++) new_stops[i] = nullptr;
}

/**
 * constructor for reading from stream
 */
reroute_train_message::reroute_train_message(
    istream& stream, bool eva_numbers, message_handler const* message_handler)
    : message_class(message_class_type::reroute_train_message) {

  unsigned int num_of_canceled_stops, num_of_new_stops;
  long seconds;

  this->category = "";

  stream >> seconds >> train_index >> num_of_canceled_stops >> num_of_new_stops;

  getline(stream, this->category, '#');

  // cout << seconds << " " << train_index << " " << num_of_canceled_stops << "
  // "
  //     << num_of_new_stops << " " << category << endl;

  release_time = message_time(seconds);

  // for stops:
  char type_char, reroute_type_char;
  message_stop::type type;
  message_stop::reroute_type reroute_type;
  int station_index;

  canceled_stops.resize(num_of_canceled_stops);
  new_stops.resize(num_of_new_stops);
  for (unsigned int i = 0; i < num_of_canceled_stops; i++)
    canceled_stops[i] = nullptr;
  for (unsigned int i = 0; i < num_of_new_stops; i++) new_stops[i] = nullptr;

  for (unsigned int i = 0; i < (num_of_canceled_stops + num_of_new_stops);
       i++) {

    stream >> type_char;
    type = type_char_to_stop_type(type_char);
    // cout << "stop-type: " << type_char << endl;

    if (eva_numbers) {
      std::string eva_number;
      stream >> eva_number;
      const station* station = message_handler->get_station_by_eva(eva_number);
      if (station == nullptr) {
        station_index = 0;
        set_parse_result(message_class::EVA_NO_CONVERSION_FAILED);
      } else {
        station_index = station->index;
      }
    } else {
      stream >> station_index;
    }

    stream >> seconds >> reroute_type_char;
    // cout << "reroute-type: " << reroute_type_char << endl;

    if (reroute_type_char == 'c') {
      // canceled stop
      reroute_type = message_stop::canceled_stop;
    } else if (reroute_type_char == 'n') {
      // new stop
      reroute_type = message_stop::new_stop;
    } else {
      // normal stop (undoing a cancellation)
      // assert (reroute_type_char == 'N');
      if (reroute_type_char != 'N')
        cout << "\n_warning(reroute_train_message): unknown reroute-type: "
             << reroute_type_char << endl;
      reroute_type = message_stop::normal;
    }

    if (i < num_of_canceled_stops)
      canceled_stops[i] = new reroute_stop(type, station_index,
                                           message_time(seconds), reroute_type);
    else
      new_stops[(i - num_of_canceled_stops)] = new reroute_stop(
          type, station_index, message_time(seconds), reroute_type);
  }
}

reroute_train_message::~reroute_train_message() {
  for (unsigned int i = 0; i < canceled_stops.size(); i++)
    delete canceled_stops[i];
  for (unsigned int i = 0; i < new_stops.size(); i++) delete new_stops[i];
}

void reroute_train_message::write_to_stream(std::ostream& output_stream) const {

  output_stream << type_to_char(type) << release_time.get_timestamp() << " "
                << train_index << " " << canceled_stops.size() << " "
                << new_stops.size() << " " << category << " ";

  reroute_stop* stop = nullptr;
  char reroute_type = '?';

  for (unsigned int i = 0; i < (canceled_stops.size() + new_stops.size());
       i++) {
    stop = (i < canceled_stops.size() ? canceled_stops[i]
                                      : new_stops[(i - canceled_stops.size())]);

    output_stream << ((stop->stop_type == message_stop::arrival ||
                       stop->stop_type == message_stop::last_stop)
                          ? 'a'
                          : 'd') << stop->station_index << " "
                  << stop->scheduled_event_time.get_timestamp() << " ";
    if (stop->reroute_type == message_stop::canceled_stop)
      reroute_type = 'c';
    else if (stop->reroute_type == message_stop::new_stop)
      reroute_type = 'n';
    if (stop->reroute_type == message_stop::normal) reroute_type = 'n';
    output_stream << reroute_type;
  }
  output_stream << "\n";
}

void reroute_train_message::debug_output(ostream& output_stream) const {

  output_stream << "message_type=" << type_to_char(type)
                << ", release_time=" << release_time.output()
                << ", tr_idx=" << train_index
                << ", num_canceled_stops: " << canceled_stops.size()
                << ", num_new_stops: " << new_stops.size()
                << ", category: " << category << endl;

  reroute_stop* stop = nullptr;
  std::string reroute_type = "?";

  output_stream << "\n_canceled stops:" << endl;

  for (unsigned int i = 0; i < (canceled_stops.size() + new_stops.size());
       i++) {
    stop = (i < canceled_stops.size() ? canceled_stops[i]
                                      : new_stops[(i - canceled_stops.size())]);

    if (i == canceled_stops.size()) output_stream << "\n_new stops:" << endl;

    output_stream << stop_type_to_string(stop->stop_type)
                  << ", st=" << stop->station_index
                  << ", t=" << stop->scheduled_event_time.output() << flush;
    if (stop->reroute_type == message_stop::canceled_stop)
      reroute_type = "canceled";
    else if (stop->reroute_type == message_stop::new_stop)
      reroute_type = "new";
    if (stop->reroute_type == message_stop::normal) reroute_type = "normal";
    output_stream << ", reroute-type=" << reroute_type << endl;
  }
  output_stream << "\n";
}

void reroute_train_message::set_stop(unsigned int const& index,
                                     message_stop::type const& type,
                                     int const& station,
                                     event_time const& scheduled,
                                     message_stop::reroute_type reroute_type) {

  if (reroute_type == message_stop::canceled_stop)
    canceled_stops[index] =
        new reroute_stop(type, station, scheduled, reroute_type);
  else
    new_stops[index] = new reroute_stop(type, station, scheduled, reroute_type);
}

bool reroute_train_message::is_equal(const reroute_train_message* other) const {

  if (this->get_type() == other->get_type() &&
      this->release_time == other->release_time &&
      this->train_index == other->train_index &&
      this->canceled_stops.size() == other->canceled_stops.size() &&
      this->new_stops.size() && other->new_stops.size())
    // TODO: sollte man alle stops überprüfen?
    return true;

  return false;
}

reroute_train_message_sg::reroute_train_message_sg(
    additional_or_canceled_train_message* additional_train_message,
    additional_or_canceled_train_message* cancel_train_message)
    : message_class(message_class_type::reroute_train_message_sg) {
  this->additional_train_message = additional_train_message;
  this->cancel_train_message = cancel_train_message;

  if (this->has_additional_train_message())
    this->release_time = this->additional_train_message->release_time;
  else if (this->has_cancel_train_message())
    this->release_time = this->cancel_train_message->release_time;
}

reroute_train_message_sg::reroute_train_message_sg(
    std::istream& input_stream, bool eva_numbers,
    message_handler const* message_handler)
    : message_class(message_class_type::reroute_train_message_sg) {

  short has_additional_train_message, has_cancel_train_message;
  input_stream >> has_additional_train_message >> has_cancel_train_message;

  if (has_additional_train_message) {
    char c;
    input_stream >> c;
    this->additional_train_message = new additional_or_canceled_train_message(
        input_stream, false, eva_numbers, message_handler);
    this->release_time = this->additional_train_message->release_time;
  } else
    this->additional_train_message = nullptr;

  if (has_cancel_train_message) {
    char c;
    input_stream >> c;
    this->cancel_train_message = new additional_or_canceled_train_message(
        input_stream, true, eva_numbers, message_handler);
    this->release_time = this->cancel_train_message->release_time;
  } else
    this->cancel_train_message = nullptr;
}

void reroute_train_message_sg::write_to_stream(
    std::ostream& output_stream) const {

  output_stream << type_to_char(this->get_type())
                << this->has_additional_train_message() << " "
                << this->has_cancel_train_message();

  if (this->has_additional_train_message())
    this->additional_train_message->write_to_stream(output_stream);

  if (this->has_cancel_train_message())
    this->cancel_train_message->write_to_stream(output_stream);

  output_stream << flush;
}

reroute_train_message_sg::~reroute_train_message_sg() {

  if (this->has_additional_train_message())
    delete this->additional_train_message;
  if (this->has_cancel_train_message()) delete this->cancel_train_message;
}

void reroute_train_message_sg::debug_output(ostream& output_stream) const {

  output_stream << "message_type=" << type_to_char(type)
                << ", release_time=" << release_time.output()
                << ", tr_idx=" << this->get_train_index()
                << "\n\n_cancel_train_message:" << endl;

  if (this->has_cancel_train_message())
    this->cancel_train_message->debug_output(output_stream);
  else
    output_stream << "no message" << endl;

  output_stream << "\n_additional_train_message:" << endl;
  if (this->has_additional_train_message())
    this->additional_train_message->debug_output(output_stream);
  else
    output_stream << "no message" << endl;
}

int reroute_train_message_sg::get_train_index() const {
  if (this->has_additional_train_message())
    return this->additional_train_message->train_index;
  else if (this->has_cancel_train_message())
    return this->cancel_train_message->train_index;
  return -1;
}

void reroute_train_message_sg::switch_messages() {
  additional_or_canceled_train_message* tmp;

  tmp = this->additional_train_message;
  this->additional_train_message = this->cancel_train_message;
  this->cancel_train_message = tmp;

  if (this->has_additional_train_message()) {
    this->additional_train_message->message_type = additional_train;
    this->additional_train_message->set_type(
        message_class_type::additional_train_message);
  }
  if (this->has_cancel_train_message()) {
    this->cancel_train_message->message_type = canceled_train;
    this->cancel_train_message->set_type(
        message_class_type::canceled_train_message);
  }
}

}  // namespace realtime
}  // namespace motis
