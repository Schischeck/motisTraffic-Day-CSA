#include "stationgeodata.h"

namespace td {
namespace railviz {

StationGeoData::StationGeoData(Schedule &schedule) :
    schedule(schedule)
{
    // organising station-to-station edges in
    // r-tree

    std::vector<StationNodePtr> &stationNodes = schedule.stationNodes;
    for( int i = 0; i < stationNodes.size(); i++ )
    {
        StationNode* node = stationNodes.at(i).get();
        std::vector<td::StationNode::StationGraphEdge> edges = node->getStationGraphEdges();

        Station* startStation = schedule.stations.at(i).get();
        point ps1( startStation->width, startStation->length );
        Station* endStation;
        for( int j = 0; j < edges.size(); j++ )
        {
            int endStationIndex = edges.at(j).first;
            endStation = schedule.stations.at(endStationIndex).get();
            point ps2( endStation->width, endStation->length );
            box bounds( ps1, ps2 );
            edgesTree.insert( std::make_pair( bounds, std::make_pair(i, endStationIndex) ) );
        }
    }
}

}}
