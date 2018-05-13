#pragma once

#include <boost/optional.hpp>
#include <memory>
#include <string>

namespace motis {
namespace path {

struct kv_database {
  virtual ~kv_database() = default;

  virtual void put(std::string const& k, std::string const& v) = 0;

  virtual std::string get(std::string const& k) const = 0;
  virtual boost::optional<std::string> try_get(std::string const& k) const = 0;
};

std::unique_ptr<kv_database> load_db(std::string const& path,
                                     bool required = false);

}  // namespace path
}  // namespace motis
