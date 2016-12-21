#include "motis/path/lookup_index.h"


namespace motis {
namespace path {

std::string lookup_index::find(PathStationSeqRequest const* req) {
  auto it = std::find_if(
      lookup_table_.get()->indices()->begin(),
      lookup_table_.get()->indices()->end(), [&](auto const& p) {
        return p->station_ids()->size() == req->station_ids()->size() &&
               std::equal(p->station_ids()->begin(), p->station_ids()->end(),
                          req->station_ids()->begin(),
                          [&](auto const& a, auto const& b) {
                            return std::strcmp(a->c_str(), b->c_str()) == 0;
                          }) &&
               std::find(p->classes()->begin(), p->classes()->end(),
                         req->clasz()) != p->classes()->end();
      });

  if (it == lookup_table_.get()->indices()->end()) {
    return "";
  }
  return std::to_string(it->index());
}

std::string lookup_index::find(std::vector<std::string> const& station_ids,
                               uint32_t const& clasz) {
  auto it = std::find_if(
      lookup_table_.get()->indices()->begin(),
      lookup_table_.get()->indices()->end(), [&](auto const& p) {
        return p->station_ids()->size() == station_ids.size() &&
               std::equal(p->station_ids()->begin(), p->station_ids()->end(),
                          station_ids.begin(),
                          [&](auto const& a, auto const& b) {
                            return std::strcmp(a->c_str(), b.c_str()) == 0;
                          }) &&
               std::find(p->classes()->begin(), p->classes()->end(), clasz) !=
                   p->classes()->end();
      });

  if (it == lookup_table_.get()->indices()->end()) {
    return "";
  }
  return std::to_string(it->index());
}

}  // namespace path
}  // namespace motis
