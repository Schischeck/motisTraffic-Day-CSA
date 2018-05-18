#include "motis/path/prepare/db_statistics.h"

#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>
#include <set>
#include <thread>

#include "geo/polyline.h"

#include "parser/util.h"

#include "utl/concat.h"
#include "utl/get_or_create.h"
#include "utl/parallel_for.h"
#include "utl/to_vec.h"

#include "motis/core/common/logging.h"
#include "motis/module/message.h"
#include "motis/module/module.h"

#include "motis/path/fbs/PathIndex_generated.h"
#include "motis/protocol/PathSeqResponse_generated.h"

namespace motis {
namespace path {

using lookup_table = typed_flatbuffer<motis::path::PathLookup>;

constexpr char kStubType[] = "STUB_ROUTE";
constexpr int kMaxDist = 5;
constexpr double kMaxAngle = 120.0;
constexpr double kMinAngle = 90.0;

bool excluded(stat_result const& result) {
  std::vector<int> excluded_class = {9, 6, 7};
  return std::find(begin(excluded_class), end(excluded_class), result.clasz_) !=
         end(excluded_class);
}

bool invalid_angles(geo::polyline const& polyline) {
  std::vector<double> bearings;
  for (auto i = 0u; i < polyline.size() - 1; ++i) {
    if (i + 1 >= polyline.size()) {
      break;
    }
    bearings.push_back(geo::bearing(polyline[i], polyline[i + 1]));
  }

  for (auto i = 0u; i < bearings.size() - 1; ++i) {
    if (i + 1 >= polyline.size()) {
      break;
    }
    auto angle1 = bearings[i] - bearings[i + 1];
    auto angle2 = bearings[i + 1] - bearings[i];
    auto angle = std::min(angle1 < 0 ? angle1 + 360 : angle1,
                          angle2 < 0 ? angle2 + 360 : angle2);
    if (angle > kMinAngle && angle <= kMaxAngle) {
      return true;
    }
  }
  return false;
}

bool btw_stations(PathSourceInfo const* current, PathSourceInfo const* next) {
  return (next == nullptr || current->segment_idx() != next->segment_idx()) &&
         current->type()->str() == kStubType;
}

bool in_station(PathSourceInfo const* current, PathSourceInfo const* next,
                geo::polyline const& polyline) {
  return (next != nullptr && (current->segment_idx() == next->segment_idx())) &&
         current->type()->str() == kStubType &&
         geo::length(polyline) >= kMaxDist;
}

stat_result::category check_category(PathSourceInfo const* current,
                                     PathSourceInfo const* next,
                                     geo::polyline const& polyline) {
  if (invalid_angles(polyline)) {
    return stat_result::category::INVALID_ANGLES;
  } else if (in_station(current, next, polyline)) {
    return stat_result::category::IN_STATIONS;
  } else if (btw_stations(current, next)) {
    return stat_result::category::BETWEEN_STATIONS;
  }
  return stat_result::category::UNKNOWN;
}

std::vector<stat_result> create_new_results(PathSeqResponse const* msg,
                                            stat_result::category category) {
  auto const& station_ids =
      utl::to_vec(msg->station_ids()->begin(), msg->station_ids()->end(),
                  [](auto const& s) { return s->str(); });

  return utl::to_vec(msg->classes()->begin(), msg->classes()->end(),
                     [&](auto const& clasz) {
                       return stat_result(clasz, category, station_ids);
                     });
}

void print_details(stat_result const& result) {
  for (auto i = 0u; i < result.station_sequences_.size(); ++i) {
    if (i == result.station_sequences_.size() - 1) {
      std::cout << result.station_sequences_[i];
    } else {
      std::cout << result.station_sequences_[i] << ".";
    }
  }
  std::cout << " [" << result.clasz_ << "] " << result.category_str()
            << std::endl;
}

void print_results(std::map<std::pair<std::string, std::string>,
                            std::vector<stat_result>> const& results,
                   bool show_details) {
  struct stats {
    int total_ = 0;
    int excluded_ = 0;
    int btw_stations_ = 0;
    int in_stations_ = 0;
    int invalid_angles_ = 0;
  } stats;

  for (auto const& pairs : results) {
    auto ex = false;
    std::set<stat_result::category> categories;
    for (auto const& result : pairs.second) {
      if (excluded(result)) {
        ex = true;
        continue;
      }

      if (show_details) {
        print_details(result);
      }

      if (categories.find(result.category_) != end(categories)) {
        continue;
      }
      categories.insert(result.category_);
    }

    for (auto const& category : categories) {
      switch (category) {
        case stat_result::category::IN_STATIONS:  //
          stats.in_stations_++;
          break;
        case stat_result::category::BETWEEN_STATIONS:
          stats.btw_stations_++;
          break;
        case stat_result::category::INVALID_ANGLES:
          stats.invalid_angles_++;
          break;
        default: break;
      }
    }
    if (categories.empty() && ex) {
      stats.excluded_++;
    }
  }

  auto const dump = [](auto& name, auto pairs) {
    std::cout << name << "\t | "
              << "count: " << std::setw(8) << pairs << std::endl;
  };

  dump("total results", results.size());
  dump("excl results", stats.excluded_);
  dump("used results", results.size() - stats.excluded_);
  dump("btw_stations", stats.btw_stations_);
  dump("in_stations", stats.in_stations_);
  dump("invalid angles", stats.invalid_angles_);
}

geo::polyline get_polyline(PathSeqResponse const* msg,
                           PathSourceInfo const* source) {
  geo::polyline poly;
  auto segment = msg->segments()->Get(source->segment_idx())->coordinates();
  for (auto i = 0u; i < segment->size() - 1; i = i + 2) {
    if (static_cast<int>(i) >= source->from_idx() &&
        static_cast<int>(i) + 1 < source->to_idx()) {
      poly.emplace_back(segment->Get(i), segment->Get(i + 1));
    }
  }
  return poly;
}

std::mutex mtx;

void check_response(PathSeqResponse const* msg,
                    std::map<std::pair<std::string, std::string>,
                             std::vector<stat_result>>& results) {
  for (auto i = 0u; i < msg->sourceInfos()->size(); ++i) {
    auto current = msg->sourceInfos()->Get(i);
    PathSourceInfo const* next = nullptr;
    if (i != msg->sourceInfos()->size() - 1) {
      next = msg->sourceInfos()->Get(i + 1);
    }

    auto const& polyline = get_polyline(msg, current);

    auto category = check_category(current, next, polyline);

    if (category != stat_result::category::UNKNOWN) {
      auto new_results = create_new_results(msg, category);
      std::pair<std::string, std::string> station_pair{
          msg->station_ids()->Get(current->segment_idx())->str(),
          msg->station_ids()->Get(current->segment_idx() + 1)->str()};

      std::lock_guard<std::mutex> lock(mtx);
      auto& value = utl::get_or_create(
          results, station_pair, [&]() { return std::vector<stat_result>(); });
      utl::concat(value, new_results);
    }
  }
}

std::map<std::pair<std::string, std::string>, std::vector<stat_result>>
check_responses(kv_database const& db,
                std::vector<std::string> const& indices) {
  std::map<std::pair<std::string, std::string>, std::vector<stat_result>>
      results;

  utl::parallel_for("checking responses", indices, 50, [&](auto const& index) {
    auto buf = db.get(index);
    auto msg_ptr = std::make_shared<module::message>(buf.size(), buf.c_str());
    auto msg = motis_content(PathSeqResponse, msg_ptr);

    check_response(msg, results);
  });
  return results;
}

void dump_db_statistics(kv_database const& db) {
  auto buf = db.try_get("__index");
  verify(buf, "could not load db");
  lookup_table lookup_table(buf->size(), buf->data());
  std::cout << "index loaded: " << lookup_table.get()->indices()->size()
            << std::endl;

  auto indices =
      utl::to_vec(lookup_table.get()->indices()->begin(),
                  lookup_table.get()->indices()->end(),
                  [&](auto const& i) { return std::to_string(i->index()); });

  auto const& results = check_responses(db, indices);
  print_results(results, true);
}

}  // namespace path
}  // namespace motis
