#pragma once

#include <ctime>
#include <memory>

#include "motis/core/schedule/time.h"
#include "motis/core/schedule/edges.h"

#include "motis/railviz/edge_geo_index.h"
#include "motis/railviz/geometry.h"
#include "motis/railviz/train.h"
#include "motis/railviz/date_converter.h"
#include "motis/core/schedule/nodes.h"

namespace motis {
namespace railviz {

typedef std::unique_ptr<std::vector<train_ptr>> train_list_ptr;

class train_query
{
public:
    train_query(edge_geo_index const&, date_converter const&);

    train_list_ptr by_bounds_and_time_interval(geometry::box bounds, std::time_t start, std::time_t end, unsigned int limit=0) const;
    std::time_t first_departure_time() const;
private:
    bool time_intervals_overlap( std::time_t t1_s, std::time_t t1_e, std::time_t t2_s, std::time_t t2_e ) const;

    edge_geo_index const& geo_index;
    date_converter const& dconv;
};

}
}
