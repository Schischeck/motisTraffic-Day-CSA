#ifndef TD_INDEX_H_
#define TD_INDEX_H_

#include "motis/core/common/offset.h"
#include "motis/core/common/array.h"

namespace td {

struct binary_schedule_index {
  typedef offset<connection*>::type connection_offset;
  typedef offset<connection_info*>::type connection_info_offset;
  typedef offset<station_node*>::type station_node_offset;
  typedef offset<station*>::type station_offset;

  binary_schedule_index(unsigned node_count, std::size_t station_count,
                        std::size_t full_connection_count,
                        std::size_t connection_info_count,
                        std::size_t station_node_count)
      : node_count(static_cast<uint32_t>(node_count)),
        stations(station_count),
        full_connections(full_connection_count),
        connection_infos(connection_info_count),
        station_nodes(station_node_count) {}

  uint32_t node_count;
  array<station_offset> stations;
  array<connection_offset> full_connections;
  array<connection_info_offset> connection_infos;
  array<station_node_offset> station_nodes;
};

}  // namespace td

#endif  // TD_INDEX_H_
