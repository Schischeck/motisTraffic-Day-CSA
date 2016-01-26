#include "motis/bikesharing/database.h"

#include <string>

#include "leveldb/db.h"

#include "motis/bikesharing/error.h"

using boost::system::system_error;

using namespace leveldb;
using namespace flatbuffers;

namespace motis {
namespace bikesharing {

struct database::database_impl {
  database_impl(std::string const& path) {
    DB* db;
    Options options;
    options.create_if_missing = true;
    Status s = DB::Open(options, path, &db);

    if (!s.ok()) {
      throw system_error(error::database_error);
    }

    db_.reset(db);
  }

  persistable_terminal get(int id) {
    std::string value;
    Status s = db_->Get(ReadOptions(), std::to_string(id), &value);

    if (s.IsNotFound()) {
      throw boost::system::system_error(error::terminal_not_found);
    } else if (!s.ok()) {
      throw system_error(error::database_error);
    }

    return value;
  }

  void put(std::vector<persistable_terminal> const& terminals) {
    for (auto const& t : terminals) {
      Status s = db_->Put(WriteOptions(), std::to_string(t.get()->id()),
                          t.to_string());

      if (!s.ok()) {
        throw system_error(error::database_error);
      }
    }
  }

  std::unique_ptr<DB> db_;
};

database::database(std::string const& path) : impl_(new database_impl(path)) {}
database::~database() = default;

persistable_terminal database::get(int id) { return impl_->get(id); }

void database::put(std::vector<persistable_terminal> const& terminals) {
  impl_->put(terminals);
}

namespace detail {

Offset<Availability> create_availability(FlatBufferBuilder& b,
                                         availability const& a) {
  return CreateAvailability(b, a.average, a.median, a.minimum, a.q90,
                            a.percent_reliable);
}

Offset<Vector<Offset<AttachedStation>>> create_attached_stations(
    FlatBufferBuilder& b, std::vector<attached_station> const& stations) {
  std::vector<Offset<AttachedStation>> vec;
  for (auto const& station : stations) {
    vec.push_back(CreateAttachedStation(b, station.id, station.duration));
  }
  return b.CreateVector(vec);
}

Offset<Vector<Offset<ReachableTerminal>>> create_reachable_terminals(
    FlatBufferBuilder& b, std::vector<reachable_terminal> const& reachables) {
  std::vector<Offset<ReachableTerminal>> vec;
  for (auto const& reachable : reachables) {
    vec.push_back(CreateReachableTerminal(b, reachable.id, reachable.duration));
  }
  return b.CreateVector(vec);
}

}  // namespace detail

persistable_terminal convert_terminal(
    terminal const& terminal, availability const& availability,
    std::vector<attached_station> const& attached,
    std::vector<reachable_terminal> const& reachable) {
  FlatBufferBuilder b;
  b.Finish(CreateTerminal(b, terminal.uid, terminal.lat, terminal.lng,
                          b.CreateString(terminal.name),
                          detail::create_availability(b, availability),
                          detail::create_attached_stations(b, attached),
                          detail::create_reachable_terminals(b, reachable)));
  return {std::move(b)};
}

}  // namespace bikesharing
}  // namespace motis
