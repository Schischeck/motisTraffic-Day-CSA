#include "motis/path/db/rocksdb.h"

#include "rocksdb/db.h"
#include "rocksdb/utilities/leveldb_options.h"

#include "motis/path/error.h"

using namespace rocksdb;

namespace motis {
namespace path {

struct rocksdb_database::impl {
  explicit impl(std::string path) {
    DB* db;
    LevelDBOptions options;
    options.create_if_missing = true;
    Status s = DB::Open(ConvertOptions(options), path, &db);

    if (!s.ok()) {
      throw std::system_error(error::database_error);
    }

    db_.reset(db);
  }

  void put(std::string const& k, std::string const& v) {
    Status s = db_->Put(WriteOptions(), k, v);
    if (!s.ok()) {
      throw std::system_error(error::database_error);
    }
  }

  std::string get(std::string const& k) {
    auto v = try_get(k);

    if (!v) {
      throw std::system_error(error::not_found);
    }

    return *v;
  }

  boost::optional<std::string> try_get(std::string const& k) {
    std::string v;
    Status s = db_->Get(ReadOptions(), k, &v);

    if (s.IsNotFound()) {
      return {};
    } else if (!s.ok()) {
      throw std::system_error(error::database_error);
    }

    return v;
  }

  std::unique_ptr<DB> db_;
};

rocksdb_database::rocksdb_database(std::string path)
    : impl_(std::make_unique<rocksdb_database::impl>(std::move(path))) {}
rocksdb_database::~rocksdb_database() = default;

void rocksdb_database::put(std::string const& k, std::string const& v) {
  impl_->put(k, v);
}

std::string rocksdb_database::get(std::string const& k) {
  return impl_->get(k);
}

boost::optional<std::string> rocksdb_database::try_get(std::string const& k) {
  return impl_->try_get(k);
}

}  // namespace path
}  // namespace motis
