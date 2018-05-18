#include "motis/path/db/lmdb.h"

#include "lmdb/lmdb.hpp"

#include "motis/path/error.h"

namespace db = lmdb;

namespace motis {
namespace path {

struct lmdb_database::impl {
  explicit impl(std::string const& path, size_t const max_db_size) {
    env_.set_maxdbs(1);
    env_.set_mapsize(max_db_size);
    env_.open(path.c_str(), db::env_open_flags::NOSUBDIR |
                                db::env_open_flags::NOLOCK |
                                db::env_open_flags::NOTLS);
  }

  void put(std::string const& k, std::string const& v) {
    auto txn = db::txn{env_};
    auto db = txn.dbi_open();
    txn.put(db, k, v);
    txn.commit();
  }

  std::string get(std::string const& k) const {
    if (auto v = try_get(k); !v) {
      return *v;
    } else {
      throw std::system_error(error::not_found);
    }
  }

  boost::optional<std::string> try_get(std::string const& k) const {
    auto txn = db::txn{env_, db::txn_flags::RDONLY};
    auto db = txn.dbi_open();
    if (auto const r = txn.get(db, k); r.has_value()) {
      return std::string{*r};
    } else {
      return {};
    }
  }

  db::env mutable env_;
};

lmdb_database::lmdb_database(std::string path, size_t const max_db_size)
    : impl_{std::make_unique<lmdb_database::impl>(std::move(path),
                                                  max_db_size)} {}

lmdb_database::~lmdb_database() = default;

void lmdb_database::put(std::string const& k, std::string const& v) {
  impl_->put(k, v);
}

std::string lmdb_database::get(std::string const& k) const {
  return impl_->get(k);
}

boost::optional<std::string> lmdb_database::try_get(
    std::string const& k) const {
  return impl_->try_get(k);
}

}  // namespace path
}  // namespace motis
