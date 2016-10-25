#pragma once

#include <set>
#include <vector>

namespace motis {
namespace routes {

struct kv_database{
	virtual kv_database(std::string file);
	virtual ~kv_database();

	virtual void put(std::string const&, std::string const&) = 0;
	virtual std::string get(std::string const&) = 0;
};

}  // namespace routes
}  // namespace motis
