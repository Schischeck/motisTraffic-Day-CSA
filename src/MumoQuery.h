#ifndef TD_MUMO_QUERY_H_
#define TD_MUMO_QUERY_H_

#include <string>

#include <pugixml.hpp>

#include "Graph.h"
#include "Arrival.h"

namespace td
{

class MumoQuery
{
  public:
    /// \param g  the graph to operate on
    MumoQuery(Graph& g);

    void init(std::string dateTime, int interval,
              Arrival dep, Arrival arr);

    /// \param response the XML document to write the response to
    bool execute(pugi::xml_node& response);

    /// \return human readable string represenation
    std::string str() const;

  private:
    /// Reference to the graph to operate on
    Graph& _graph;

    /// Search day index.
    int _day_index;

    /// Start time of the search interval.
    int _time1;

    /// End time of the search interval.
    int _time2;

    /// Date.
    int _day, _month, _year;

    /// Start time.
    int _h1, _m1;

    /// End time.
    int _h2, _m2;

    /// Source location -> first station(s) information.
    Arrival _dep;

    /// Destination station(s) -> destination location information.
    Arrival _arr;

    /// Indicates whether this query object is initialized or not.
    /// After a successful call of MumoQuery::initFromXML it is initialized
    /// and therefore ready to be executed.
    bool _initialized;
};

}  // namespace td

#endif  // TD_MUMO_QUERY_H_
