#include "motis/path/prepare/db_statistics.h"

#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

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

void dump_db_statistics(kv_database const& db) {
  auto buf = db.try_get("__index");
  if (!buf) {
    std::cout << "could not load index" << std::endl;
  }
  lookup_table lookup_table(buf->size(), buf->data());
  std::cout << "index loaded: " << lookup_table.get()->indices()->size()
            << std::endl;

  auto indices =
      utl::to_vec(lookup_table.get()->indices()->begin(),
                  lookup_table.get()->indices()->end(),
                  [&](auto const& i) { return std::to_string(i->index()); });
  std::mutex mtx;
  std::map<std::pair<std::string, std::string>,
           std::vector<std::pair<int, std::vector<std::string>>>>
      result;
  utl::parallel_for("checking results", indices, 10, [&](auto const& index) {

    auto buf = db.get(index);
    auto msg_ptr = std::make_shared<module::message>(buf.size(), buf.c_str());
    auto msg = motis_content(PathSeqResponse, msg_ptr);

    for (auto i = 0u; i < msg->sourceInfos()->size() - 1; ++i) {

      if ((msg->sourceInfos()->Get(i)->segment_idx() !=
               msg->sourceInfos()->Get(i + 1)->segment_idx() ||
           i == msg->sourceInfos()->size() - 1) &&
          msg->sourceInfos()->Get(i)->type()->str() == kStubType) {

        auto station_pair = std::make_pair<std::string, std::string>(
            msg->station_ids()
                ->Get(msg->sourceInfos()->Get(i)->segment_idx())
                ->str(),
            msg->station_ids()
                ->Get(msg->sourceInfos()->Get(i)->segment_idx() + 1)
                ->str());

        auto station_ids =
            utl::to_vec(msg->station_ids()->begin(), msg->station_ids()->end(),
                        [](auto const& s) { return s->str(); });
        std::lock_guard<std::mutex> lock(mtx);
        auto& value = utl::get_or_create(result, station_pair, [&]() {
          return std::vector<std::pair<int, std::vector<std::string>>>();
        });
        std::for_each(
            msg->classes()->begin(), msg->classes()->end(),
            [&](auto const& c) { value.emplace_back(c, station_ids); });
        break;
      }
    }
  });
  auto not_unknown = 0;
  for (auto const& seq : result) {
    for (auto const& path : seq.second) {
      if (path.first != 9) {
        not_unknown++;
        for (auto i = 0u; i < path.second.size(); ++i) {
          if (i == path.second.size() - 1) {
            std::cout << path.second[i];
          } else {
            std::cout << path.second[i] << ".";
          }
        }
        std::cout << " [" << path.first << "]" << std::endl;
      }
      break;
    }
  }

  std::cout << "station pairs connected with stub routes: " << result.size()
            << std::endl;
  std::cout << "station pairs connected with stub routes not class 9: "
            << not_unknown << std::endl;
}

}  // namespace path
}  // namespace motis
