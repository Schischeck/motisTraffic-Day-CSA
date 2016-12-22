#include "motis/path/prepare/rel/polyline_cache.h"

#include "parser/file.h"

#include "utl/to_vec.h"

#include "motis/path/fbs/AggregatedPolylines_generated.h"

using namespace parser;
using namespace flatbuffers;

namespace motis {
namespace path {

std::vector<aggregated_polyline> load_relation_polylines(
    std::string const& filename) {
  auto const buf = file{filename.c_str(), "r"}.content();
  return utl::to_vec(
      *GetAggregatedPolylines(buf.buf_)->entries(),
      [](auto const& polyline) -> aggregated_polyline {
        auto const& fbs_source = polyline->source();
        return {{fbs_source->id(),
                 static_cast<source_spec::category>(fbs_source->category()),
                 static_cast<source_spec::type>(fbs_source->type())},
                utl::to_vec(*polyline->polyline(), [](auto const& pos) {
                  return geo::latlng{pos->lat(), pos->lng()};
                })};
      });
}

void store_relation_polylines(
    std::string const& filename,
    std::vector<aggregated_polyline> const& polylines) {
  FlatBufferBuilder fbb;
  auto const fbs_polylines = utl::to_vec(polylines, [&fbb](
                                                        auto const& polyline) {
    auto const& source = polyline.source_;
    SourceSpec fbs_source{source.id_, static_cast<int64_t>(source.category_),
                          static_cast<int64_t>(source.type_)};
    auto const positions = utl::to_vec(polyline.polyline_, [](auto const& pos) {
      return Position(pos.lat_, pos.lng_);
    });

    return CreateAggregatedPolyline(fbb, &fbs_source,
                                    fbb.CreateVectorOfStructs(positions));
  });

  fbb.Finish(CreateAggregatedPolylines(fbb, fbb.CreateVector(fbs_polylines)));
  file{filename.c_str(), "w+"}.write(fbb.GetBufferPointer(), fbb.GetSize());
}

}  // namespace path
}  // namespace motis
