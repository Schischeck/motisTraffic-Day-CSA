#include "motis/bikesharing/database.h"

#include "leveldb/db.h"

#include "motis/bikesharing/error.h"

using boost::system::system_error;

using namespace leveldb;
using namespace flatbuffers;

namespace motis {
namespace bikesharing {

constexpr auto kSummaryKey = "__summary";

struct database::database_impl {
  database_impl() = default;

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

  virtual ~database_impl() = default;

  virtual persistable_terminal get(std::string const& id) const {
    std::string value;
    Status s = db_->Get(ReadOptions(), id, &value);

    if (s.IsNotFound()) {
      throw boost::system::system_error(error::terminal_not_found);
    } else if (!s.ok()) {
      throw system_error(error::database_error);
    }

    return value;
  }

  virtual void put(std::vector<persistable_terminal> const& terminals) {
    for (auto const& t : terminals) {
      Status s = db_->Put(WriteOptions(), t.get()->id()->str(), t.to_string());
      if (!s.ok()) {
        throw system_error(error::database_error);
      }
    }
  }

  virtual bikesharing_summary get_summary() const {
    std::string value;
    Status s = db_->Get(ReadOptions(), kSummaryKey, &value);

    if (s.IsNotFound() || !s.ok()) {
      throw system_error(error::database_error);
    }

    return value;
  }

  virtual void put_summary(bikesharing_summary const& summary) {
    Status s = db_->Put(WriteOptions(), kSummaryKey, summary.to_string());
    if (!s.ok()) {
      throw system_error(error::database_error);
    }
  }

  std::unique_ptr<DB> db_;
};

struct inmemory_database : public database::database_impl {
  persistable_terminal get(std::string const& id) const {
    auto it = store_.find(id);
    if (it == end(store_)) {
      throw boost::system::system_error(error::terminal_not_found);
    }
    return it->second;
  }

  void put(std::vector<persistable_terminal> const& terminals) {
    for (auto const& t : terminals) {
      store_[t.get()->id()->str()] = t.to_string();
    }
  }

  bikesharing_summary get_summary() const {
    auto it = store_.find(kSummaryKey);
    if (it == end(store_)) {
      throw boost::system::system_error(error::terminal_not_found);
    }
    return it->second;
  }

  void put_summary(bikesharing_summary const& summary) {
    store_[kSummaryKey] = summary.to_string();
  }

  std::map<std::string, std::string> store_;
};

database::database(std::string const& path)
    : impl_(path == ":memory:" ? new inmemory_database()
                               : new database_impl(path)) {}
database::~database() = default;

persistable_terminal database::get(std::string const& id) const {
  return impl_->get(id);
}

void database::put(std::vector<persistable_terminal> const& terminals) {
  impl_->put(terminals);
}

bikesharing_summary database::get_summary() const {
  return impl_->get_summary();
}

void database::put_summary(bikesharing_summary const& summary) {
  impl_->put_summary(summary);
}

namespace detail {

Offset<Vector<Offset<Availability>>> create_availabilities(
    FlatBufferBuilder& b, hourly_availabilities const& availabilities) {
  std::vector<Offset<Availability>> vec;
  for (auto const& a : availabilities) {
    vec.push_back(CreateAvailability(b, a.average, a.median, a.minimum, a.q90,
                                     a.percent_reliable));
  }
  return b.CreateVector(vec);
}

Offset<Vector<Offset<CloseLocation>>> create_close_locations(
    FlatBufferBuilder& b, std::vector<close_location> const& locations) {
  std::vector<Offset<CloseLocation>> vec;
  for (auto const& l : locations) {
    vec.push_back(CreateCloseLocation(b, b.CreateString(l.id), l.duration));
  }
  return b.CreateVector(vec);
}

}  // namespace detail

persistable_terminal convert_terminal(
    terminal const& terminal, hourly_availabilities const& availabilities,
    std::vector<close_location> const& attached,
    std::vector<close_location> const& reachable) {
  FlatBufferBuilder b;
  b.Finish(CreateTerminal(b, b.CreateString(terminal.uid), terminal.lat,
                          terminal.lng, b.CreateString(terminal.name),
                          detail::create_availabilities(b, availabilities),
                          detail::create_close_locations(b, attached),
                          detail::create_close_locations(b, reachable)));
  return {std::move(b)};
}

bikesharing_summary make_summary(std::vector<terminal> const& terminals) {
  FlatBufferBuilder b;

  std::vector<Offset<TerminalLocation>> locations;
  for (auto const& terminal : terminals) {
    locations.push_back(CreateTerminalLocation(b, b.CreateString(terminal.uid),
                                               terminal.lat, terminal.lng));
  }
  b.Finish(CreateSummary(b, b.CreateVector(locations)));

  return {std::move(b)};
}

}  // namespace bikesharing
}  // namespace motis
