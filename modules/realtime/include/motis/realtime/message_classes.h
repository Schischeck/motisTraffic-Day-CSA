#pragma once

#include <ctime>
#include <string>
#include <iostream>
#include <vector>

#include "motis/core/schedule/time.h"
#include "motis/core/schedule/date_manager.h"

/**
 * short summary:
 *
 * classes for stops of trains:
 * - schedule_stop: day, station, type and scheduled time
 * - delayed_stop: additionally a new time (may be a forecast)
 *
 * classes for messages:
 * - delayed_train_message
 *   a delayed train, its first message (is-message) and all its forecasts
 * - additional_or_canceled_train_message
 *   all stops of an additional train or all canceled stops of a train
 *   (complete/partial cancelation)
 * - connection_status_decision_message
 *   two trains, the station and the decision kept, unkept, or new_connection
 * - reroute_train_message
 *   canceled and new stops for an existing train (for rerouting a train)
 *
 * all lists of stops are in their natural order:
 *  ordered by time, arrivals before departures at the same station
 *
 * a station_index is our internal index (obtained from eva or ds100 key)
 * use information in xxx.stations.txt to map keys to internal index
 *
 * author: mathias schnee
 * contact: schnee@cs.tu-darmstadt.de
 */

namespace motis {
namespace realtime {

class message_handler;

/******************************************************************************/
/* definition of abstract superclass for all messages */
/******************************************************************************/
/// namespace for the types of messages {
namespace message_class_type {
/**
 * message types
 */
enum type {
  /// message for delayed trains
  delayed_train_message,
  /// message for additional trains
  additional_train_message,
  /// message for canceled trains
  canceled_train_message,
  /// message for connection status decision
  connection_status_decision_message,
  /// message for connection status decisions with trains at different stations
  connection_status_decision_message_different_stations,

  /// message for a connection status assessment from DB
  connection_status_assessment_db_message,
  /// message for a connection status assessment from DB with trains at
  /// different stations
  connection_status_assessment_db_message_different_stations,

  /// message for rerouting a train
  reroute_train_message,
  /// message for rerouting a train (used for search graph)
  reroute_train_message_sg,
  /// to use as default and for error handling
  no_message,

  /// to conveniently obtain the number of different types
  HIGHST_MESSAGE_TYPE = no_message
};
}

class message_time {
public:
  message_time() : _timestamp(0) {}
  message_time(time_t const& timestamp) : _timestamp(timestamp) {}

  bool operator==(message_time const& other) const {
    return _timestamp == other._timestamp;
  }

  std::time_t get_timestamp() const { return _timestamp; }

  time to_time(date_manager const& date_mgr) const {
    std::tm* t = std::localtime(&_timestamp);
    if (t == nullptr) return INVALID_TIME;
    motis::date_manager::date date(t->tm_mday, t->tm_mon + 1,
                                   t->tm_year + 1900);
    int minutes = t->tm_hour * 60 + t->tm_min;
    int day_index = date_mgr.get_day_index(date);
    if (day_index != date_manager::NO_INDEX)
      return motis::to_time(day_index, minutes);
    else
      return INVALID_TIME;
  }

  std::string output() const {
    char buf[100];
    if (std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S",
                      std::localtime(&_timestamp)))
      return buf;
    else
      return "";
  }

  friend std::ostream& operator<<(std::ostream& os, message_time const& mt) {
    os << mt.output();
    return os;
  }

private:
  std::time_t _timestamp;
};

/**
 * abstract superclass for all messages
 */
class message_class {
protected:
  /**
   * protected constructor:
   * do not need to be public because a abstract class
   * can only be instantiated by its child classes
   */
  message_class(message_class_type::type message_type,
                const message_time& release)
      : type(message_type), release_time(release) {}

  /**
   * protected constructor for read from stream initialization
   * do not need to be public because a abstract class
   * can only be instantiated by its child classes
   */
  message_class(message_class_type::type message_type) : type(message_type) {}

public:
  /**
   * virtual destructor makes it to a abstract class
   */
  virtual ~message_class() {}

  /**
   * read from input stream
   */
  /**
   * read message from input stream (deserialize)
   *
   * @param input_stream stream to read from
   */
  static message_class* read_message_from_stream(
      std::istream& input_stream, bool eva_numbers,
      message_handler const* message_handler);

  /**
   * output message to stream (serialize to ascii)
   */
  virtual void write_to_stream(std::ostream& output_stream) const;

  message_class_type::type get_type() const { return type; }

  void set_type(message_class_type::type msg_type) { type = msg_type; }

  const message_time& get_release_time() const { return release_time; }

  static std::string to_string(message_class_type::type type) {
    switch (type) {
      case message_class_type::delayed_train_message:
        return "delayed train";
      /*
      case message_class_type::additional_or_canceled_train_message:
        return "additional or canceled train";
      */
      case message_class_type::additional_train_message:
        return "additional train";
      case message_class_type::canceled_train_message: return "canceled train";
      case message_class_type::connection_status_decision_message:
        return "connection status decision";
      case message_class_type::
          connection_status_decision_message_different_stations:
        return "connection status decision (different stations)";
      case message_class_type::connection_status_assessment_db_message:
        return "connection status assessment";
      case message_class_type::
          connection_status_assessment_db_message_different_stations:
        return "connection status assessment (different stations)";
      case message_class_type::reroute_train_message: return "reroute train";
      case message_class_type::reroute_train_message_sg:
        return "reroute train (SG)";
      case message_class_type::no_message: return "no message";
    }
    return "unknown";
  }

  static message_class_type::type char_to_type(char c) {
    switch (c) {
      case 'D': return message_class_type::delayed_train_message;
      case 'A': return message_class_type::additional_train_message;
      case 'C': return message_class_type::canceled_train_message;
      case 'S': return message_class_type::connection_status_decision_message;
      case 's':
        return message_class_type::
            connection_status_decision_message_different_stations;
      case 'B':
        return message_class_type::connection_status_assessment_db_message;
      case 'b':
        return message_class_type::
            connection_status_assessment_db_message_different_stations;
      case 'R': return message_class_type::reroute_train_message;
      case 'r': return message_class_type::reroute_train_message_sg;
    }
    // default - should never be reached
    return message_class_type::no_message;
  }

  static char type_to_char(message_class_type::type type) {
    switch (type) {
      case message_class_type::delayed_train_message: return 'D';
      case message_class_type::additional_train_message: return 'A';
      case message_class_type::canceled_train_message: return 'C';
      case message_class_type::connection_status_decision_message: return 'S';
      case message_class_type::
          connection_status_decision_message_different_stations:
        return 's';
      case message_class_type::connection_status_assessment_db_message:
        return 'B';
      case message_class_type::
          connection_status_assessment_db_message_different_stations:
        return 'b';
      case message_class_type::reroute_train_message: return 'R';
      case message_class_type::reroute_train_message_sg: return 'r';
      case message_class_type::no_message: return 'N';
    }
    return '?';
  }

  enum parse_result {
    /// parsed OK
    OK,
    /// eva_number could not be converted
    EVA_NO_CONVERSION_FAILED,
    /// problem parsing
    PARSE_ERROR,
    /// type neither departure nor arrival
    UNKNOWN_STOP_TYPE
  };

  /** write the message to a stream (human readable) */
  virtual void debug_output(std::ostream& output_stream) const = 0;

  /** get train-index */
  // virtual int get_train_index() const = 0;

  static bool parse_error_occured();

  static void set_parse_result(parse_result result);

  static void reset_parse_result();

  /// type of the message
  message_class_type::type type;

  /// release time of the message
  message_time release_time;

  /// parse error occured
  static parse_result _parse_result;
};

/******************************************************************************/
/* definition of stops and constants */
/******************************************************************************/

/**
 * time specifying when a stop event occured (scheduled or delayed)
 */
typedef message_time event_time;

/// namespace for the stops in a message {
namespace message_stop {

/** message stop type
 * departure or arrival stop
 * additionally begin or end stop
 */
enum type {
  /// departure
  departure,
  /// arrival
  arrival,
  /// first stop of train (departure)
  first_stop,
  /// last stop of train (arrival)
  last_stop
};

/**
 * enum for reroute-stops
 */
enum reroute_type {
  /// for canceled stops
  canceled_stop,
  /// for new stops in the route
  new_stop,
  /// for taking back a cancellation
  normal
};

/**
 *
 */
enum timestamp_reason_type {
  /// time from schedule
  schedule,
  /// time from forecast
  forecast,
  /// time from is-message
  is_message,
  /// time from propagation (e.g. wait for feeder at this or earlier stop,
  /// forecast or is_message for earlier stop)
  propagation
};
}

std::string timestamp_reason_to_string(
    message_stop::timestamp_reason_type const& reason);

/**
 * convert given message_stop::type to string (departure, arrival, begin, or
 * end)
 */
std::string stop_type_to_string(message_stop::type type);
message_stop::type type_char_to_stop_type(char type);

/**
 * stop class for additional or canceled trains
 * consisting of type, station and scheduled time
 */
struct schedule_stop {
  /**
   * constructor
   * @param type type of stop (departure or arrival)
   * @param station index of station
   * @param scheduled_time scheduled time of event
   */
  schedule_stop(message_stop::type type, int station, event_time scheduled_time)
      : stop_type(type),
        station_index(station),
        scheduled_event_time(scheduled_time) {}

  /// departure or arrival
  message_stop::type stop_type;

  /** internal index of station
   * (converted from DS100 using xxx.stations.txt)
   */
  int station_index;

  /// time in minutes after midnight
  event_time scheduled_event_time;
};

/**
 * standard_stop with a new event time
 * consisting of type, station, scheduled and new time
 */
struct delayed_stop : public schedule_stop {
  /**
   * constructor
   * @param type type of stop (departure or arrival)
   * @param station index of station
   * @param scheduled_time scheduled time of event
   * @param new_event_time new time of event
   */
  delayed_stop(message_stop::type const& type, int const& station,
               event_time const& scheduled_time, event_time const& new_time)
      : schedule_stop(type, station, scheduled_time),
        new_event_time(new_time) {}

  /// new time
  event_time new_event_time;
};

/**
 * standard_stop with a attribute reroute_type
 * consisting of type, station, scheduled and new time
 */
struct reroute_stop : public schedule_stop {
  /**
   * constructor
   * @param type type of stop (departure or arrival)
   * @param station index of station
   * @param scheduled_time scheduled time of event
   * @param new_event_time new time of event
   */
  reroute_stop(message_stop::type const& type, int const& station,
               event_time const& scheduled_time,
               message_stop::reroute_type const& reroute_type)
      : schedule_stop(type, station, scheduled_time),
        reroute_type(reroute_type) {}

  /// reroute-type (new_stop or normal)
  message_stop::reroute_type reroute_type;
};

/******************************************************************************/
/* definition of message type for a delayed train */
/******************************************************************************/
/**
 * sequence of stop_messages,
 * - starts with an already happened event (delayed or not)
 * - has an arbitrary number of forecast_messages (may be zero)
 * sometimes a number of forecase_messages is created without an already
 * happened event (no such data or somewhere else than at a station)
 * use the second constructor in that case!
 *
 */
class delayed_train_message : public message_class {
public:
  /**
   * constructor
   * initialized with the already happened event, train, and times
   * @param number_of_forecasts number of forecasts for this delayed train
   * @param type type of stop (departure or arrival)
   * @param station index of station
   * @param scheduled scheduled time of event
   * @param new_event_time new time of event
   * @param train index of train
   * @param release release time of message(s)
   */
  delayed_train_message(unsigned int const& number_of_forecasts,
                        message_stop::type const& type, int const& station,
                        event_time const& scheduled_time,
                        event_time const& new_event_time, int train,
                        message_time const& release)
      : message_class(message_class_type::delayed_train_message, release),
        train_index(train),
        forecasts(number_of_forecasts) {
    delay = new delayed_stop(type, station, scheduled_time, new_event_time);
    for (unsigned int i = 0; i < number_of_forecasts; i++)
      forecasts[i] = nullptr;
  }

  /**
   * constructor without an already happened event
   * @param number_of_forecasts number of forecasts for this delayed train
   * @param train index of train
   * @param release release time of message(s)
   */
  delayed_train_message(unsigned int const& number_of_forecasts, int train,
                        message_time const& release)
      : message_class(message_class_type::delayed_train_message, release),
        train_index(train),
        forecasts(number_of_forecasts) {
    delay = nullptr;
    for (unsigned int i = 0; i < number_of_forecasts; i++)
      forecasts[i] = nullptr;
  }

  delayed_train_message(std::istream& stream, bool eva_numbers,
                        message_handler const* message_handler);

  /**
  * destructor
  */
  ~delayed_train_message() {
    if (delay) delete delay;
    for (unsigned int i = 0; i < forecasts.size(); i++) {
      if (forecasts[i]) delete forecasts[i];
    }
  }

  /**
   * adds the next forecast for the delayed train
   * @param index index of the forecast (starts at 0)
   * @param type type of stop (departure or arrival)
   * @param station index of station
   * @param scheduled_time scheduled time of event
   * @param new_event_time new time of event
   */
  void set_forecast(unsigned int const& index, message_stop::type const& type,
                    int const& station, event_time const& scheduled_time,
                    event_time const& new_event_time) {
    forecasts[index] =
        new delayed_stop(type, station, scheduled_time, new_event_time);
  }

  bool contains_happened_event() const { return delay != nullptr; }

  /**
   * output message to stream (serialize to ascii)
   */
  void write_to_stream(std::ostream& output_stream) const;

  /** write the message to a stream (human readable) */
  void debug_output(std::ostream& output_stream) const;

  int get_train_index() const { return train_index; }

  /// external train index (number)
  int train_index;

  /**
   * message about a stop that already happend (delayed or not)
   */
  delayed_stop* delay;

  /**
   * message(s) about a forecast for a stop (delayed or not)
   * always ordered from first to last
   */
  std::vector<delayed_stop*> forecasts;
};

/******************************************************************************/
/* definition of message type for additional and canceled trains */
/******************************************************************************/

/// types for train messages
enum train_message_type {
  // additional train
  additional_train,
  // canceled train
  canceled_train
};

/**
 * a message about an additional or canceled train
 * (depending on message_type)
 */
class additional_or_canceled_train_message : public message_class {
public:
  /**
   * constructor
   * @param type additional train or canceled train
   * @param number_of_stops number of additional or canceled stops
   * @param train index of the train
   * @param release release time of message(s)
   */
  additional_or_canceled_train_message(train_message_type type,
                                       unsigned int const& number_of_stops,
                                       int train, message_time const& release);

  /**
   * constructor for reading from stream
   */
  additional_or_canceled_train_message(std::istream& input_stream,
                                       bool is_canceled_train, bool eva_numbers,
                                       message_handler const* message_handler);

  /**
   * destructor
   */
  ~additional_or_canceled_train_message();

  /**
   * set category of additional train (irrelevant for canceled train)
   */
  void set_category(std::string traincategory) { category = traincategory; }

  /**
   * get category of additional train (irrelevant for canceled train)
   */
  std::string get_category() const { return category; }

  /**
   * output message to stream (serialize to ascii)
   */
  void write_to_stream(std::ostream& output_stream) const;

  /**
   * append the next stop to the train
   * @param type type of stop (departure or arrival)
   * @param station index of station
   * @param scheduled_time scheduled time of event
   */
  void set_stop(unsigned int const& index, message_stop::type const& type,
                int const& station, event_time const& scheduled);
  void set_stop(unsigned int const& index, schedule_stop* stop);

  bool is_equal(const additional_or_canceled_train_message* other) const;

  /**
   * this method returns true if the events are not
   * in the correct order according to the stop-types.
   * @return bool
   */
  bool events_are_inconsistent() const;

  /** write the message to a stream (human readable) */
  void debug_output(std::ostream& output_stream) const;

  int get_train_index() const { return train_index; }

  /// additional oder canceled train message
  train_message_type message_type;

  /// external train index (number)
  int train_index;

  /// category (of additional trains only)
  std::string category;

  /** stops of the train
   * always ordered from first to last
   */
  std::vector<schedule_stop*> stops;
};

/******************************************************************************/
/* connection status decisions */
/******************************************************************************/

/// namespace for connection status decisions
namespace connection_status {

/// types for connection status decisions
enum decision_type {
  /// connection will hold (be kept at all costs)
  kept,

  /// connection will break (will not be kept)
  unkept,

  /// new connection (one that was not originally considered a connection)
  new_connection,

  /// neutral value (only for initilization)
  undecided
};

#define ASSESSMENT_NOT_PRESENT 0
typedef int assessment_class;
}

/**
 * class describing an interchange info element
 * consisting of one or two stations and the arriving and departing train
 */
class interchange_info_element {
public:
  interchange_info_element() {}

  interchange_info_element(int const& station_from, int const& station_to,
                           int const& train_from, int const& train_to,
                           event_time const& train_from_arrival_time,
                           event_time const& train_to_departure_time)
      : station_from_index(station_from),
        station_to_index(station_to),
        train_from_index(train_from),
        train_to_index(train_to),
        train_from_time(train_from_arrival_time),
        train_to_time(train_to_departure_time) {}

  interchange_info_element get_interchange_info_for_map() {
    message_time dummy_time;
    return interchange_info_element(station_from_index, station_to_index,
                                    train_from_index, train_to_index,
                                    dummy_time, dummy_time);
  }

  bool operator<(interchange_info_element const& other) const {
    if (station_from_index != other.station_from_index)
      return station_from_index < other.station_from_index;

    if (station_to_index != other.station_to_index)
      return station_to_index < other.station_to_index;

    if (train_from_index != other.train_from_index)
      return train_from_index < other.train_from_index;

    // if (train_to_index != other.train_to_index) return
    return train_to_index < other.train_to_index;
  }

  /**
   * initialize reading from stream
   * @param input_stream stream to read message from
   * @param different_stations boolean specifying whether or not the train
   * change
   * is between different stations
   * @param eva_numbers are stations given using eva_number or station index
   */
  void init_from_stream(std::istream& input_stream, bool different_stations,
                        bool eva_numbers,
                        message_handler const* message_handler);

  /**
   * output message to stream (serialize to ascii)
   */
  void write_to_stream(std::ostream& output_stream,
                       bool different_stations) const;

  bool describes_same_interchange(interchange_info_element const& other) const;

  /// station of feeding/arriving train
  int station_from_index;

  /// index of station of connecting/departing train
  int station_to_index;

  /// index of feeding/arriving train
  int train_from_index;

  /// index of connecting/departing train
  int train_to_index;

  /// scheduled arrival time of train_from
  event_time train_from_time;

  /// scheduled departure time of train_to
  event_time train_to_time;
};

/**
 * base class for connection status decision messages (kept, unkept, new...)
 * and DB connection status rating (1..5)
 * this class is used to encapsulate the station and train information in these
 * messages
 */
class connection_status_info : public message_class {
protected:
  /**
   * constructor
   */
  connection_status_info(message_class_type::type type)
      : message_class(type, message_time(0)) {}

public:
  /**
   * constructor
   */
  connection_status_info(message_class_type::type type, int const& station_from,
                         int const& station_to, int const& train_from,
                         int const& train_to,
                         event_time const& train_from_arrival_time,
                         event_time const& train_to_departure_time,
                         message_time const& release)
      : message_class(type, release),
        interchange_info(station_from, station_to, train_from, train_to,
                         train_from_arrival_time, train_to_departure_time) {}

  /**
   * initialize reading from stream
   * @param input_stream stream to read message from
   * @param different_stations boolean specifying whether or not the train
   * change
   * is between different stations
   * @param eva_numbers are stations given using eva_number or station index
   */
  void init_from_stream(std::istream& input_stream, bool different_stations,
                        bool eva_numbers,
                        message_handler const* message_handler);

  /// get index of station of feeding/arriving train
  /// or both trains if no station change is involved
  int get_station_from_index() const {
    return interchange_info.station_from_index;
  }

  /// get index of station of connecting/departing train
  int get_station_to_index() const { return interchange_info.station_to_index; }

  /// get index of feeding train
  int get_train_from_index() const { return interchange_info.train_from_index; }

  /// get arrival time of feeding train
  event_time get_train_from_time() const {
    return interchange_info.train_from_time;
  }

  /// get departure time of connecting train
  int get_train_to_index() const { return interchange_info.train_to_index; }

  /// get index of connecting train
  event_time get_train_to_time() const {
    return interchange_info.train_to_time;
  }

  // int get_train_index() const  	  	{ return train_from_index; }

  /// reads the status decision or assessment
  virtual void read_and_parse_additional_info(std::istream& input_stream) = 0;

  /// returns the status decision or assessment for output
  virtual std::string write_additional_info() const = 0;

  /**
   * output message to stream (serialize to ascii)
   */
  void write_to_stream(std::ostream& output_stream) const;

  /** write the message to a stream (human readable) */
  void debug_output(std::ostream& output_stream) const;

  /**
   * this method compares all attributes except status and release-time
   * of this message with another message.
   *
   * @param connection_status_info* other
   * @return bool
   */
  bool describes_same_interchange(connection_status_info* other) const;

  interchange_info_element interchange_info;
};

/**
 * a message about a connection status decision.
 * both trains stop at the same station (type
 * connection_status_decision_message)
 * or at different stations, e.g. change between berlin hbf and berlin hbf
 * (tief)
 * (type connection_status_decision_message_different_stations)
 */
class connection_status_decision_message : public connection_status_info {
public:
  /**
   * constructor
   * for train change at a station
   * @param station index of station
   * @param train_from index of feeding/arriving train
   * @param train_to index of connecting/departing train
   * @param train_from_arrival_time scheduled arrival time of train_from
   * @param train_to_departure_time scheduled departure time of train_to
   * @param release release time of message(s)
   */
  connection_status_decision_message(
      connection_status::decision_type const& status_type, int const& station,
      int const& train_from, int const& train_to,
      event_time const& train_from_arrival_time,
      event_time const& train_to_departure_time, message_time const& release)
      : connection_status_info(
            message_class_type::connection_status_decision_message, station,
            station, train_from, train_to, train_from_arrival_time,
            train_to_departure_time, release),
        status(status_type) {}

  /**
   * constructor
   * for train change between two stations
   * @param station_from index of station of feeding/arriving train
   * @param station_to index of station of connecting/departing train
   * @param train_from index of feeding/arriving train
   * @param train_to index of connecting/departing train
   * @param train_from_arrival_time scheduled arrival time of train_from
   * @param train_to_departure_time scheduled departure time of train_to
   * @param release release time of message(s)
   */
  connection_status_decision_message(
      connection_status::decision_type const& status_type,
      int const& station_from, int const& station_to, int const& train_from,
      int const& train_to, event_time const& train_from_arrival_time,
      event_time const& train_to_departure_time, message_time const& release)
      : connection_status_info(
            message_class_type::
                connection_status_decision_message_different_stations,
            station_from, station_to, train_from, train_to,
            train_from_arrival_time, train_to_departure_time, release),
        status(status_type) {}

  /**
   * constructor for reading from stream
   * @param input_stream stream to read message from
   * @param different_stations boolean specifying whether or not the train
   * change
   * is between different stations
   * @param eva_numbers are stations given using eva_number or station index
   */
  connection_status_decision_message(std::istream& input_stream,
                                     bool different_stations, bool eva_numbers,
                                     message_handler const* message_handler)
      : connection_status_info(
            different_stations
                ? message_class_type::
                      connection_status_decision_message_different_stations
                : message_class_type::connection_status_decision_message) {
    init_from_stream(input_stream, different_stations, eva_numbers,
                     message_handler);
  }

  /**
   * empty destructor
   */
  ~connection_status_decision_message() {}

  /// reads the status decision
  void read_and_parse_additional_info(std::istream& input_stream);

  /// writes the status decision
  std::string write_additional_info() const;

  std::string get_status_string() const;

  /// type of the decision
  connection_status::decision_type status;
};

/**
 * a message about a connection status assessment.
 * both trains stop at the same station (type
 * connection_status_assessment_db_message)
 * or at different stations, e.g. change between berlin hbf and berlin hbf
 * (tief)
 * (type connection_status_assessment_db_message_different_stations)
 */
class connection_status_assessment_db_message : public connection_status_info {
public:
  /**
   * constructor
   * for train change at a station
   * @param station index of station
   * @param train_from index of feeding/arriving train
   * @param train_to index of connecting/departing train
   * @param train_from_arrival_time scheduled arrival time of train_from
   * @param train_to_departure_time scheduled departure time of train_to
   * @param release release time of message(s)
   */
  connection_status_assessment_db_message(
      connection_status::assessment_class status_assessment, int const& station,
      int const& train_from, int const& train_to,
      event_time const& train_from_arrival_time,
      event_time const& train_to_departure_time, message_time const& release)
      : connection_status_info(
            message_class_type::connection_status_assessment_db_message,
            station, station, train_from, train_to, train_from_arrival_time,
            train_to_departure_time, release),
        assessment(status_assessment) {}

  /**
   * constructor
   * for train change between two stations
   * @param station_from index of station of feeding/arriving train
   * @param station_to index of station of connecting/departing train
   * @param train_from index of feeding/arriving train
   * @param train_to index of connecting/departing train
   * @param train_from_arrival_time scheduled arrival time of train_from
   * @param train_to_departure_time scheduled departure time of train_to
   * @param release release time of message(s)
   */
  connection_status_assessment_db_message(
      connection_status::assessment_class status_assessment,
      int const& station_from, int const& station_to, int const& train_from,
      int const& train_to, event_time const& train_from_arrival_time,
      event_time const& train_to_departure_time, message_time const& release)
      : connection_status_info(
            message_class_type::
                connection_status_assessment_db_message_different_stations,
            station_from, station_to, train_from, train_to,
            train_from_arrival_time, train_to_departure_time, release),
        assessment(status_assessment) {}

  /**
   * constructor for reading from stream
   * @param input_stream stream to read message from
   * @param different_stations boolean specifying whether or not the train
   * change
   * is between different stations
   * @param eva_numbers are stations given using eva_number or station index
   */
  connection_status_assessment_db_message(
      std::istream& input_stream, bool different_stations, bool eva_numbers,
      message_handler const* message_handler)
      : connection_status_info(
            different_stations
                ? message_class_type::
                      connection_status_assessment_db_message_different_stations
                : message_class_type::connection_status_assessment_db_message) {
    init_from_stream(input_stream, different_stations, eva_numbers,
                     message_handler);
  }

  /**
   * empty destructor
   */
  ~connection_status_assessment_db_message() {}

  /// reads the status assessment
  void read_and_parse_additional_info(std::istream& input_stream);

  /// writes the status assessment
  std::string write_additional_info() const;

  /// the assessment
  connection_status::assessment_class assessment;
};

/**
 * a message about a reroute of a train.
 * reroute-messages are used when a train doesn't
 * stop at some station and also if it stops at new stations.
 * a reroute-message is also sent for undoing a train-cancellation.
 */
class reroute_train_message : public message_class {

public:
  /**
   * constructor
   * @param number_of_canceled_stops number of canceled stops
   * @param number_of_new_stops number of new stops
   * @param train index of the train
   * @param release release time of message(s)
   */
  reroute_train_message(unsigned int const& number_of_canceled_stops,
                        unsigned int const& number_of_new_stops, int train,
                        std::string const& category,
                        message_time const& release);

  /**
   * constructor for reading from stream
   */
  reroute_train_message(std::istream& input_stream, bool eva_numbers,
                        message_handler const* message_handler);

  /**
   * destructor
   */
  ~reroute_train_message();

  /**
   * output message to stream (serialize to ascii)
   */
  void write_to_stream(std::ostream& output_stream) const;

  /** write the message to a stream (human readable) */
  void debug_output(std::ostream& output_stream) const;

  /**
   * append the next stop to the train
   * @param index of stop
   * @param type type of stop (departure or arrival)
   * @param station index of station
   * @param scheduled_time scheduled time of event
   * @param reroute-type (canceled_stop, new_stop, normal)
   */
  void set_stop(unsigned int const& index, message_stop::type const& type,
                int const& station, event_time const& scheduled,
                message_stop::reroute_type reroute_type);

  bool is_equal(const reroute_train_message* other) const;

  int get_train_index() const { return train_index; }

  /** canceled stops of the train
   * always ordered from first to last
   */
  std::vector<reroute_stop*> canceled_stops;

  /** new stops of the train
   * always ordered from first to last
   */
  std::vector<reroute_stop*> new_stops;

  /// train-index
  int train_index;

  /// category
  std::string category;
};

/**
 * this is an internal message type and is used for
 * transmitting reroute-train messages to the search graph.
 * this message type is used because reroute-train messages
 * are processed by DG-server. this process results in
 * a train-cancellation and a additional-train message.
 * reroute_train_message_sg contains these two messages.
 */
class reroute_train_message_sg : public message_class {

public:
  /**
   * constructor
   *
   * additional_or_canceled_train_message* additional_train_message
   * additional_or_canceled_train_message* cancel_train_message
   */
  reroute_train_message_sg(
      additional_or_canceled_train_message* additional_train_message,
      additional_or_canceled_train_message* cancel_train_message);

  /**
   * constructor for reading from stream
   */
  reroute_train_message_sg(std::istream& input_stream, bool eva_numbers,
                           message_handler const* message_handler);

  /** destructor */
  ~reroute_train_message_sg();

  /**
   * output message to stream (serialize to ascii)
   */
  void write_to_stream(std::ostream& output_stream) const;

  /** get additional-train message */
  const additional_or_canceled_train_message* get_additional_train_message(
      void) const {
    return additional_train_message;
  }

  /** get cancel-train message */
  const additional_or_canceled_train_message* get_cancel_train_message() const {
    return cancel_train_message;
  }

  /** write the message to a stream (human readable) */
  void debug_output(std::ostream& output_stream) const;

  /** true if message contains an additional-train message */
  bool has_additional_train_message() const {
    return (additional_train_message != nullptr);
  }

  /** true if message contains a cancel-train message */
  bool has_cancel_train_message() const {
    return (cancel_train_message != nullptr);
  }

  /** get the external train index */
  int get_train_index() const;

  /** this method is called when queue simulator is in rewinding mode
   * and we have to undo a rerouting message */
  void switch_messages();

private:
  /** additional-train message */
  additional_or_canceled_train_message* additional_train_message;
  /** cancel-train message */
  additional_or_canceled_train_message* cancel_train_message;
};

}  // namespace realtime
}  // namespace motis