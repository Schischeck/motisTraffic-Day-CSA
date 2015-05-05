#ifndef TD_QUERY_H_
#define TD_QUERY_H_

#include <pugixml.hpp>

#include <string>
#include <tuple>
#include <stdexcept>

#include "Graph.h"
#include "StationGuesser.h"

namespace td
{

/// Exception that gets thrown on parser errors.
/// For example: missing (non-optional) attributes or malformed XML
class parse_exception : public std::runtime_error {
 public:
  parse_exception(const std::string& msg) : std::runtime_error(msg) {}
};

/// The Query class is responible for
///   - Parsing the XML query
///   - Executing the query
///   - Creating and returning the XML formatted query result
///
/// Sample input:
/// \code{.xml}
/// <Query>
///   <Interval definitionFor="departure">
///     <Begin dateTime="${INTERVAL_START}"/>
///     <End   dateTime="${INTERVAL_END}"/>
///   </Interval>
///   <PathDescription>
///     <Station EvaNo="${SOURCE_EVA_NO}"/>
///     <Section/>
///     <Station EvaNo="${TARGET_EVA_NO}"/>
///   </PathDescription>
///   <AdditionalOptionList/>
/// </Query>
/// \endcode
///
/// Currently not supported:
///   - Backward search (definitionFor="arrival")
///   - Section categories
///   - AttributeList and ExcludedTrainList in Section
///   - Additional options
///   - Via search
class Query
{
  public:
    /// Tuple of 5 integer values representing a point in time.
    typedef std::tuple<int /* year */,
                       int /* month */,
                       int /* day */,
                       int /* hour */,
                       int /* minute */> time;

    /// Easy access constants for the time tuple.
    enum { YEAR, MONTH, DAY, HOUR, MINUTE };

    /// \param g  the graph to operate on
    Query(Graph& g, const StationGuesser& stationGuesser);

    /// Initialize a query from an XML Query node string.
    ///
    /// \param doc  the parsed XML query string
    /// \throws     parse_exception on parse errors (attributes not found, ...)
    void initFromXML(const pugi::xml_document& query,
                     bool oneHourInterval);

    /// \param response the XML document to write the response to
    bool execute(pugi::xml_document& response);

    /// \return human readable string represenation
    const std::string& str() const
    { return _stringRepresentation; }

    /// Reads the time from the given string.
    /// Example input: 2013-03-26T15:55
    ///
    /// \param time_str the string to read the dateTime from
    /// \throws parse_exception if the input has the wrong format
    static time read_time(const std::string& time_str);

    /// Retrievs the station index from the given XML input:
    ///
    /// Sample input:
    /// \code{.xml}
    ///   <Station EvaNo="${EVA_NO}"/>
    /// \endcode
    ///
    /// \code{.xml}
    ///   <Station name="${STATION_NAME}"/>
    /// \endcode
    ///
    /// \code{.xml}
    ///   <Station name="${STATION_NAME}" guess="true"/>
    /// \endcode
    ///
    /// If the guess flag is set, the station guesser will be used to
    /// match the exact station name if the station name is cannot be found.
    /// Warning: This can lead to unexpected results.
    ///
    /// \throws parse_exception if the station name could not be resolved or
    ///                         if the eva number is not available
    const Station* readStation(const pugi::xml_node& station);

  private:
    /// Reference to the graph to operate on
    Graph& _graph;

    /// Reference to the station guesser
    const StationGuesser& _stationGuesser;

    /// The query stations.
    int _from, _to;

    /// Search day index.
    int _day_index;

    /// Start time of the search interval.
    int _time1;

    /// End time of the search interval.
    int _time2;

    /// Indicates whether this query object is initialized or not.
    /// After a successful call of Query::initFromXML it is initialized
    /// and therefore ready to be executed.
    bool _initialized;

    /// Human readable representation.
    std::string _stringRepresentation;
};

}  // namespace td

#endif  // TD_QUERY_H_
