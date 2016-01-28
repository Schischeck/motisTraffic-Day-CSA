#include "motis/bikesharing/nextbike_initializer.h"

#include <functional>
#include <memory>

#include "boost/algorithm/string/predicate.hpp"
#include "boost/filesystem.hpp"

#include "pugixml.hpp"

#include "parser/arg_parser.h"
#include "parser/file.h"

#include "motis/bikesharing/error.h"
#include "motis/core/common/constants.h"
#include "motis/core/common/geo.h"
#include "motis/core/common/logging.h"
#include "motis/core/common/util.h"
#include "motis/module/message.h"

#include "motis/protocol/Message_generated.h"

using namespace flatbuffers;
using namespace motis::intermodal;
using namespace motis::geo_detail;
using namespace motis::logging;
using namespace motis::module;
using namespace parser;
using namespace pugi;
namespace fs = boost::filesystem;
using fs::directory_iterator;
using boost::system::error_code;

namespace motis {
namespace bikesharing {

std::vector<std::string> get_nextbike_files(std::string const& nextbike_path) {
  fs::path b{nextbike_path};
  if (!fs::exists(b)) {
    return {};
  } else if (fs::is_regular_file(b)) {
    return {b.string()};
  } else if (fs::is_directory(b)) {
    std::vector<std::string> files;
    for (auto it = directory_iterator(b); it != directory_iterator(); ++it) {
      if (!fs::is_regular_file(it->status())) {
        continue;
      }

      auto filename = it->path().string();
      if (boost::algorithm::iends_with(filename, ".xml")) {
        files.push_back(filename);
      }
    }
    return files;
  }
  return {};
}

std::time_t nextbike_filename_to_timestamp(std::string const& filename) {
  auto dash_pos = filename.rfind("-");
  auto dot_pos = filename.rfind(".");
  if (dash_pos == std::string::npos || dot_pos == std::string::npos ||
      dash_pos > dot_pos) {
    throw std::runtime_error("unexpected nextbike filename");
  }

  return std::stoul(filename.substr(dash_pos + 1, dot_pos - dash_pos - 1));
}

std::vector<terminal_snapshot> nextbike_parse_xml(parser::buffer&& buffer) {
  std::vector<terminal_snapshot> result;

  xml_document d;
  d.load_buffer_inplace(reinterpret_cast<void*>(buffer.data()), buffer.size());

  constexpr char const* query = "/markers/country[@country='DE']/city/place";
  for (auto const& xnode : d.select_nodes(query)) {
    auto const& node = xnode.node();

    terminal_snapshot terminal;
    terminal.uid = node.attribute("uid").as_int();
    terminal.lat = node.attribute("lat").as_double();
    terminal.lng = node.attribute("lng").as_double();
    terminal.name = node.attribute("name").value();
    terminal.available_bikes = parse<int>(node.attribute("bikes").value(), 0);
    result.push_back(terminal);
  }

  return result;
}

struct context {
  database* db_;
  dispatch_fun dispatch_fun_;
  module::callback finished_cb_;

  std::vector<terminal> terminals_;
  std::vector<hourly_availabilities> availabilities_;

  std::vector<std::vector<close_location>> attached_stations_;
  std::vector<std::vector<close_location>> reachable_terminals_;
};
using ctx_ptr = std::shared_ptr<context>;

msg_ptr terminals_to_geo_request(std::vector<terminal> const& terminals,
                                 double radius);
void initialize_nextbike(ctx_ptr ctx, std::string const& nextbike_path);
void find_close_terminals(ctx_ptr ctx);
void handle_attached_stations(ctx_ptr ctx, msg_ptr msg, error_code ec);
void persist_terminals(ctx_ptr ctx);

void initialize_nextbike(database* db, dispatch_fun dispatch_fun,
                         module::callback finished_cb,
                         std::string const& nextbike_path) {
  auto ctx = std::make_shared<context>();
  ctx->db_ = db;
  ctx->dispatch_fun_ = dispatch_fun;
  ctx->finished_cb_ = finished_cb;
  return initialize_nextbike(ctx, nextbike_path);
}

void initialize_nextbike(ctx_ptr ctx, std::string const& nextbike_path) {
  auto files = get_nextbike_files(nextbike_path);
  LOG(info) << "loading " << files.size() << " NEXTBIKE XML files";

  manual_timer parse_timer("NEXTBIKE parsing");
  snapshot_merger merger;
  for (auto const& filename : files) {
    auto xml_string = parser::file(filename.c_str(), "r").content();
    auto timestamp = nextbike_filename_to_timestamp(filename);
    auto snapshot = nextbike_parse_xml(std::move(xml_string));
    merger.add_snapshot(timestamp, snapshot);
  }
  parse_timer.stop_and_print();

  manual_timer merge_timer("NEXTBIKE merging");
  auto merged = merger.merged();
  merge_timer.stop_and_print();

  ctx->terminals_ = merged.first;
  ctx->availabilities_ = merged.second;

  auto req = terminals_to_geo_request(merged.first, MAX_WALK_DIST);
  namespace p = std::placeholders;
  return ctx->dispatch_fun_(
      req, std::bind(handle_attached_stations, ctx, p::_1, p::_2));
}

void handle_attached_stations(ctx_ptr ctx, msg_ptr msg, error_code ec) {
  if (ec) {
    return ctx->finished_cb_({}, ec);
  }

  for (size_t i = 0; i < ctx->terminals_.size(); ++i) {
    auto const& t = ctx->terminals_[i];

    auto const& content = msg->content<IntermodalGeoIndexResponse const*>();
    auto const& found_stations = content->coordinates()->Get(i)->stations();
    std::vector<close_location> attached_stations;
    for (auto const& station : *found_stations) {
      int dist = distance_in_m(t.lat, t.lng, station->lat(), station->lng());
      attached_stations.push_back({station->eva()->str(), dist});
    }

    ctx->attached_stations_.push_back(attached_stations);
  }

  return find_close_terminals(ctx);
}

void find_close_terminals(ctx_ptr ctx) {
  std::vector<value> values;
  for (size_t i = 0; i < ctx->terminals_.size(); ++i) {
    values.push_back({{ctx->terminals_[i].lng, ctx->terminals_[i].lat}, i});
  }
  bgi::rtree<value, bgi::quadratic<16>> rtree{values};

  for (size_t i = 0; i < ctx->terminals_.size(); ++i) {
    auto const& t = ctx->terminals_[i];
    spherical_point t_location(t.lng, t.lat);

    std::vector<close_location> reachable_terminals;
    for (auto it = rtree.qbegin(
             bgi::intersects(generate_box(t_location, MAX_BIKE_DIST)) &&
             bgi::satisfies([&t_location](value const& v) {
               return distance_in_m(v.first, t_location) < MAX_BIKE_DIST;
             }));
         it != rtree.qend(); ++it) {
      int dist = distance_in_m(it->first, t_location);
      reachable_terminals.push_back({ctx->terminals_[it->second].uid, dist});
    }
    ctx->reachable_terminals_.push_back(reachable_terminals);
  }

  return persist_terminals(ctx);
}

void persist_terminals(ctx_ptr ctx) {
  std::vector<persistable_terminal> p;
  for (size_t i = 0; i < ctx->terminals_.size(); ++i) {
    p.push_back(convert_terminal(ctx->terminals_[i], ctx->availabilities_[i],
                                 ctx->attached_stations_[i],
                                 ctx->reachable_terminals_[i]));
  }
  ctx->db_->put(p);
  ctx->db_->put_summary(make_summary(ctx->terminals_));
  ctx->finished_cb_({}, error::ok);
}

msg_ptr terminals_to_geo_request(std::vector<terminal> const& terminals,
                                 double radius) {
  MessageCreator b;
  std::vector<Offset<intermodal::RequestCoordinates>> c;
  for (auto const& merged : terminals) {
    c.push_back(CreateRequestCoordinates(b, merged.lat, merged.lng, radius));
  }
  b.CreateAndFinish(MsgContent_IntermodalGeoIndexRequest,
                    b.CreateVector(c).Union());
  return make_msg(b);
}

}  // namespace bikesharing
}  // namespace motis
