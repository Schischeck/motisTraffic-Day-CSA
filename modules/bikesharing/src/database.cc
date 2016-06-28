#include "motis/bikesharing/database.h"

#include "rocksdb/db.h"
#include "rocksdb/utilities/leveldb_options.h"

#include "motis/bikesharing/error.h"

using std::system_error;

using namespace rocksdb;
using namespace flatbuffers;

namespace motis {
namespace bikesharing {

constexpr auto kSummaryKey = "__summary";

struct database::database_impl {
  database_impl() = default;

  explicit database_impl(std::string const& path) {
    DB* db;
    LevelDBOptions options;
    options.create_if_missing = true;
    Status s = DB::Open(ConvertOptions(options), path, &db);

    if (!s.ok()) {
      throw system_error(error::database_error);
    }

    db_.reset(db);
  }

  virtual ~database_impl() = default;

  virtual bool is_initialized() const {
    std::string value;
    Status s = db_->Get(ReadOptions(), kSummaryKey, &value);

    return s.ok() && !s.IsNotFound();
  }

  virtual persistable_terminal get(std::string const& id) const {
    std::string value;
    Status s = db_->Get(ReadOptions(), id, &value);

    if (s.IsNotFound()) {
      throw system_error(error::terminal_not_found);
    } else if (!s.ok()) {
      throw system_error(error::database_error);
    }

    return persistable_terminal(value);
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

    return bikesharing_summary(value);
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

  bool is_initialized() const override {
    auto it = store_.find(kSummaryKey);
    return it != end(store_);
  }

  persistable_terminal get(std::string const& id) const override {
    auto it = store_.find(id);
    if (it == end(store_)) {
      throw system_error(error::terminal_not_found);
    }
    return persistable_terminal(it->second);
  }

  void put(std::vector<persistable_terminal> const& terminals) override {
    for (auto const& t : terminals) {
      store_[t.get()->id()->str()] = t.to_string();
    }
  }

  bikesharing_summary get_summary() const override {
    auto it = store_.find(kSummaryKey);
    if (it == end(store_)) {
      throw system_error(error::database_not_initialized);
    }
    return bikesharing_summary(it->second);
  }

  void put_summary(bikesharing_summary const& summary) override {
    store_[kSummaryKey] = summary.to_string();
  }

  std::map<std::string, std::string> store_;
};

database::database(std::string const& path)
    : impl_(path == ":memory:" ? new inmemory_database()
                               : new database_impl(path)) {}
database::~database() = default;

bool database::is_initialized() const {
  return impl_->is_initialized();
}

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
    vec.push_back(CreateAvailability(b, a.average_, a.median_, a.minimum_,
                                     a.q90_, a.percent_reliable_));
  }
  return b.CreateVector(vec);
}

Offset<Vector<Offset<CloseLocation>>> create_close_locations(
    FlatBufferBuilder& b, std::vector<close_location> const& locations) {
  std::vector<Offset<CloseLocation>> vec;
  for (auto const& l : locations) {
    vec.push_back(CreateCloseLocation(b, b.CreateString(l.id_), l.duration_));
  }
  return b.CreateVector(vec);
}

}  // namespace detail

persistable_terminal convert_terminal(
    terminal const& terminal, hourly_availabilities const& availabilities,
    std::vector<close_location> const& attached,
    std::vector<close_location> const& reachable) {
  FlatBufferBuilder b;
  b.Finish(CreateTerminal(b, b.CreateString(terminal.uid_), terminal.lat_,
                          terminal.lng_, b.CreateString(terminal.name_),
                          detail::create_availabilities(b, availabilities),
                          detail::create_close_locations(b, attached),
                          detail::create_close_locations(b, reachable)));
  return persistable_terminal(std::move(b));
}

bikesharing_summary make_summary(std::vector<terminal> const& terminals) {
  FlatBufferBuilder b;

  std::vector<Offset<TerminalLocation>> locations;
  for (auto const& terminal : terminals) {
    locations.push_back(CreateTerminalLocation(b, b.CreateString(terminal.uid_),
                                               terminal.lat_, terminal.lng_));
  }
  b.Finish(CreateSummary(b, b.CreateVector(locations)));

  return bikesharing_summary(std::move(b));
}

}  // namespace bikesharing
}  // namespace motis
