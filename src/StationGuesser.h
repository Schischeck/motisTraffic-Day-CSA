#ifndef TD_STATION_GUESSER_H_
#define TD_STATION_GUESSER_H_

// Maximum length where matches should be cut
#define TRIM 4

#include <vector>
#include <string>
#include <unordered_map>

#include "Station.h"

namespace td
{

class Graph;

/// Station guesser class.
///
/// Able to produce station guesses for given (fuzzy) user inputs.
class StationGuesser
{
  public:
    /// \param graph the graph containing the reference stations
    StationGuesser(std::vector<StationPtr> const& stations);

    /// \param name  the name of the station to return
    /// \return the station if found or nullptr if not
    Station const* getStationByName(std::string const& name) const;

    /// \param evaNr  the eva number of the station to return
    /// \return the station if found or nullptr if not
    Station const* getStationByEva(std::string const& evaNr) const;

    /// \param evaNr  the eva number of the station to return
    /// \return the station if found or nullptr if not
    Station const* getStationByEva(int evaNr) const;

    /// Returns station guesses for the given input.
    ///
    /// \param input  the input string to find guesses for
    /// \param count  the count of guesses to generate
    /// \return a list of guesses
    std::vector<const Station*> guess(const std::string& input,
                                      const std::size_t count) const;

    /// Writes statistics with the following format:
    /// index evaNr %ev1 ev2 ... ev10 heuristic
    ///
    /// All stations are sorted by their heuristic value.
    ///
    /// \param filename  path of the file to write
    void writeStatistics(std::string const& filename) const;

  private:
    /// Normalizes the string:
    /// lower case, replaces umlauts, special chars to space
    ///
    /// \param s the stirng to normalize
    static void normalize(std::string& s);

    /// Calculates a heuristic using the similarity and the station usage
    /// (count of arrival/departure events at this station).
    ///
    /// \param similarity   the similarity measure (using 3grams)
    /// \param statioIndex  the station to compare with
    /// \return the calculated heuristic value
    double stationHeuristic(double similarity, int stationIndex) const;

    /// Similarity measure.
    ///
    /// \param ref        the reference station name (from the station list)
    /// \param candidate  the candidate to check for
    /// \return the similiarity heuristic (using trigram overlap count)
    double similarity(int stationIndex,
                      const std::vector<std::string>& tRef) const;

    /// Generates a trigram vector for the given input
    ///
    /// \param  input the input to split into trigrams
    /// \return the trigrams for the given input
    static std::vector<std::string> trigrams(const std::string& input);

    /// Builds the regular expression that filters all stations to a list
    /// of candidates that will be tested with the trigram heuristic.
    ///
    /// Example input: "abcd"
    /// Example output: "((^|[ \.])[ab].?[bc].?[cd].*)"
    ///
    /// \param s  input string to build the regular expression for
    /// \return the regular expression
    static std::string build_regex(const std::string& s);

    /// All stations.
    std::vector<StationPtr> const& _stations;

    /// Mapping from station eva number to station.
    std::unordered_map<int, const Station*> _evaStationMap;

    /// Mapping from station name to station.
    std::unordered_map<std::string, const Station*> _nameStationMap;

    /// Normalized (lower case, "  " -> " " replaced) station names.
    std::vector<std::string> _normalizedStationNames;

    /// Precomputed trigrams of all stations.
    std::vector<std::vector<std::string>> _stationTrigrams;

    /// Station size factor (depends on the arrival/departure count).
    std::vector<double> _stationSizes;
};

}  // namespace td

#endif  // TD_STATION_GUESSER_H_
