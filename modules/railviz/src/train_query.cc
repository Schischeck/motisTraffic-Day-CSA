#include "motis/railviz/train_query.h"

namespace motis {
namespace railviz {

train_query::train_query(edge_geo_index const& geo_index , const date_converter &dconv) :
    geo_index(geo_index), dconv(dconv) {}

train_list_ptr train_query::by_bounds_and_time_interval(geometry::box bounds,
                                                        std::time_t start,
                                                        std::time_t end,
                                                        unsigned int limit) const
{
    geometry::point top_left = bounds.top_left();
    geometry::point bottom_right = bounds.bottom_right();
    train_list_ptr train_list(new std::vector<train_ptr>);
    std::vector<const motis::edge*> edges = geo_index.edges(bottom_right.lat, bottom_right.lng,
                                                            top_left.lat, top_left.lng);

    int start_station_id, end_station_id;
    std::time_t start_time, end_time;
    for( int clasz = 0; clasz < 10; clasz++ )
    {
        // iteration breaks after trains-amount-limit is reached
        if( limit > 0 )
            if( train_list.get()->size() >= limit )
                break;

        for( auto* edge : edges )
        {
            // iteration breaks after trains-amount-limit is reached
            if( limit > 0 )
                if( train_list.get()->size() >= limit )
                    break;

            if( edge->type() != motis::edge::ROUTE_EDGE )
                continue;

            // Iteration skipps every connection whitch is not of type
            // clasz
            if( edge->_m._route_edge._conns._used_size > 0 )
                if( edge->_m._route_edge._conns[0]._full_con->clasz != clasz )
                    continue;

            start_station_id = edge->_from->get_station()->_id;
            end_station_id = edge->_to->get_station()->_id;

            for( auto const& lcon : edge->_m._route_edge._conns )
            {
                // iteration breaks after trains-amount-limit is reached
                if( limit > 0 )
                    if( train_list.get()->size() >= limit )
                        break;
                start_time = dconv.convert(lcon.d_time);
                end_time = dconv.convert(lcon.a_time);
                if(time_intervals_overlap(start, end, start_time, end_time))
                {
                    train* train_p = new train;
                    train_p->d_station = start_station_id;
                    train_p->a_station = end_station_id;
                    train_p->d_time = start_time;
                    train_p->a_time = end_time;
                    train_ptr train(train_p);
                    train_list.get()->push_back( std::move(train) );
                }
            }
        }

    }
    return std::move(train_list);
}

bool train_query::time_intervals_overlap(std::time_t t1_s, std::time_t t1_e, std::time_t t2_s, std::time_t t2_e) const
{
    if( t1_s <= t2_s && t2_s <= t1_e ||
            t1_s <= t2_e && t2_e <= t1_e ||
            t2_s <= t1_s && t1_s <= t2_e ||
            t2_s <= t1_e && t1_e <= t2_e )
        return true;
    return false;
}

}
}
