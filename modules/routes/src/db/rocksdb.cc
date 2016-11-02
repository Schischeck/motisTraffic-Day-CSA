#include "motis/routes/db/rocksdb.h"

#include "rocksdb/db.h"
#include "rocksdb/utilities/leveldb_options.h"

#include "motis/routes/error.h"

using namespace rocksdb;

namespace motis {
namespace routes {

struct rocksdb_database::impl {
  impl(std::string path) {
    DB* db;
    LevelDBOptions options;
    options.create_if_missing = true;
    Status s = DB::Open(ConvertOptions(options), path, &db);

    if (!s.ok()) {
      throw std::system_error(error::database_error);
    }

    db_.reset(db);
  }

  void put(std::string const& key, std::string const& value) {
    Status s = db_->Put(WriteOptions(), key, value);
    if (!s.ok()) {
      throw std::system_error(error::database_error);
    }
  }

  std::string get(std::string const& key) {
    auto value = try_get(key);

    if (!value) {
      throw std::system_error(error::not_found);
    }

    return *value;
  }

  boost::optional<std::string> try_get(std::string const& key) {
    std::string value;
    Status s = db_->Get(ReadOptions(), key, &value);

    if (s.IsNotFound()) {
      return {};
    } else if (!s.ok()) {
      throw std::system_error(error::database_error);
    }

    return value;
  }

  std::unique_ptr<DB> db_;
};

rocksdb_database::rocksdb_database(std::string path)
    : impl_(std::make_unique<rocksdb_database::impl>(std::move(path))) {}
rocksdb_database::~rocksdb_database() = default;

void rocksdb_database::put(std::string const& key, std::string const& value) {
  impl_->put(key, value);
}

std::string rocksdb_database::get(std::string const& key) {
  return impl_->get(key);
}

boost::optional<std::string> rocksdb_database::try_get(std::string const& key) {
  return impl_->try_get(key);
}

}  // namespace routes
}  // namespace motis
