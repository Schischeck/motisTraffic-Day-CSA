#include "motis/reliability/db_distributions_loader.h"

#include <climits>
#include <utility>

#include <boost/lexical_cast.hpp>

#include "parser/csv.h"

#include "motis/reliability/db_distributions.h"

namespace motis {
namespace reliability {
namespace db_distributions_loader {

void load_distributions(
    std::string root, unsigned int const max_expected_travel_time,
    unsigned int const max_expected_departure_delay,
    std::map<std::string, std::string>& family_to_distribution_class,
    std::vector<std::pair<unsigned int, probability_distribution> >&
        probability_distributions,
    std::vector<resolved_mapping>& distribution_mappings,
    std::map<std::string, probability_distribution>&
        class_to_probability_distributions) {
  if (root.length() == 0) {
    root = "./";
  }
  if (root[root.length() - 1] != '/') {
    root += '/';
  }
  detail::load_distributions_classes(root + "Classes.csv",
                                     family_to_distribution_class);
  detail::load_distributions(root + "Distributions.csv",
                             probability_distributions);
  detail::load_distribution_mappings(
      root + "Mapping.csv", max_expected_travel_time,
      max_expected_departure_delay, distribution_mappings);
  detail::load_start_distributions(root + "StartDistributions.csv",
                                   class_to_probability_distributions);
}

namespace detail {

/************************** data types *************************/

/* distribution classes */
using classes_csv = std::tuple<std::string, std::string>;
parser::column_mapping<classes_csv> const classes_columns = {
    {"Zuggattung", "Zuggattungsklasse"}};
enum classes_pos { c_family, c_distribution_class };

/* distributions */
using distributions_csv = std::tuple<int, int, std::string>;
parser::column_mapping<distributions_csv> const distributions_columns = {
    {"KLASSEN_ID", "verspaetungsDelta", "p_Verspaetungsdelta"}};
enum distributions_pos {
  d_distribution_id,
  d_delay_minute,
  d_delay_probability
};

/* distribution mapping */
using mapping_csv = std::tuple<int, std::string, std::string, std::string,
                               std::string, std::string>;
parser::column_mapping<mapping_csv> const mapping_columns = {
    {"KLASSEN_ID", "ZUGGATTUNGSKLASSE", "VORSCHAUZEITKLASSE_VON",
     "VORSCHAUZEITKLASSE_BIS", "AKTUELLEVERSPAETUNGSLAGE_VON",
     "AKTUELLEVERSPAETUNGSLAGE_BIS"}};
enum mapping_pos {
  m_distribution_id,
  m_distribution_class,
  m_from_travel_time,
  m_to_travel_time,
  m_from_delay,
  m_to_delay
};

/* start distributions */
using start_distributions_csv = std::tuple<std::string, int, std::string>;
parser::column_mapping<start_distributions_csv> const
    start_distributions_columns = {
        {"Zuggattungsklasse", "Startverspaetung", "p_startverspaetung"}};
enum start_distributions_pos {
  s_distribution_class,
  s_delay_minute,
  s_delay_probability
};

/************************** helper functions *************************/

template <typename DistributionIDType>
struct distribution_info {
  distribution_info(DistributionIDType const distribution_id,
                    int const first_min)
      : distribution_id_(distribution_id), first_min_(first_min) {}
  DistributionIDType const distribution_id_;
  int const first_min_;
  std::vector<probability> probabilities_;
};

template <typename Tuple, unsigned int DistributionIDPos,
          unsigned int DelayMinutePos>
bool distribution_is_smaller(Tuple a, Tuple b) {
  if (std::get<DistributionIDPos>(a) < std::get<DistributionIDPos>(b))
    return true;
  return std::get<DistributionIDPos>(a) == std::get<DistributionIDPos>(b) &&
         std::get<DelayMinutePos>(a) < std::get<DelayMinutePos>(b);
}

template <typename EnteriesType, unsigned int DelayMinutePos,
          unsigned int DelayProbabilityPos>
void store_probability(std::vector<EnteriesType> const& distributions_entries,
                       unsigned int const entry_index,
                       bool const is_first_distribution_element,
                       std::vector<probability>& probabilities) {
  /* In the distributions-file, minutes with a probability equal 0 are ignored.
   * In order to initialize a probability_distribution object, we need
   * a vector containing the probabilities of all minutes without gaps. */
  if (!is_first_distribution_element) {
    unsigned int const num_missing_enteries =
        (std::get<DelayMinutePos>(distributions_entries[entry_index]) -
         std::get<DelayMinutePos>(distributions_entries[entry_index - 1])) -
        1;
    for (unsigned int i = 0; i < num_missing_enteries; i++)
      probabilities.push_back(0.0);
  }
  probability const prob = std::stod(
      std::get<DelayProbabilityPos>(distributions_entries[entry_index]));
  assert(smaller_equal(prob, 1.0));
  probabilities.push_back(prob);
}

template <typename EnteriesType, typename DistributionIDType,
          unsigned int DistributionIDPos, unsigned int DelayMinutePos,
          unsigned int DelayProbabilityPos>
void parse_distributions(
    std::vector<EnteriesType> const& distributions_entries,
    std::vector<std::pair<DistributionIDType, probability_distribution> >&
        probability_distributions) {
  std::vector<distribution_info<DistributionIDType> > distribution_infos;

  // convert distributions_entries to distribution_infos
  for (unsigned int i = 0; i < distributions_entries.size(); i++) {
    bool const is_first_distribution_element =
        (i == 0 ||
         std::get<DistributionIDPos>(distributions_entries[i]) !=
             std::get<DistributionIDPos>(distributions_entries[i - 1]));

    // add new distribution entry if necessary
    if (is_first_distribution_element) {
      distribution_infos.emplace_back(
          (DistributionIDType)std::get<DistributionIDPos>(
              distributions_entries[i]),
          std::get<DelayMinutePos>(
              distributions_entries[i])); /* first delay minute */
    }

    store_probability<EnteriesType, DelayMinutePos, DelayProbabilityPos>(
        distributions_entries, i, is_first_distribution_element,
        distribution_infos[distribution_infos.size() - 1].probabilities_);
  }

  // convert distribution_infos to probability_distributions
  probability_distributions.resize(distribution_infos.size());
  for (unsigned int i = 0; i < distribution_infos.size(); i++) {
    probability_distributions[i].first = distribution_infos[i].distribution_id_;
    probability_distributions[i].second.init(
        distribution_infos[i].probabilities_, distribution_infos[i].first_min_);
    assert(equal(probability_distributions[i].second.sum(), 1.0));
  }
}

inline bool parse_integer(std::string const& str, int& number) {
  try {
    number = boost::lexical_cast<int>(str);
    return true;
  } catch (const boost::bad_lexical_cast&) {
    return false;
  }
}

inline unsigned int map_negative_to_zero(int number) {
  return (unsigned int)std::max(0, number);
}

bool parse_travel_time_interval(std::string const& from_travel_time_str,
                                std::string const& to_travel_time_str,
                                unsigned int const max_expected_travel_time,
                                unsigned int& from_travel_time,
                                unsigned int& to_travel_time) {
  int to_travel_time_int;
  if (!parse_integer(to_travel_time_str, to_travel_time_int)) {
    to_travel_time = max_expected_travel_time;
  } else {
    if (to_travel_time_int < 0) {
      return false;
    } else if ((unsigned int)to_travel_time_int > max_expected_travel_time) {
      to_travel_time = max_expected_travel_time;
    } else /* to_travel_time_int <= max_expected_travel_time */ {
      to_travel_time = (unsigned int)to_travel_time_int;
    }
  }

  int from_travel_time_int;
  if (!parse_integer(from_travel_time_str, from_travel_time_int)) {
    return false;
  }
  from_travel_time =
      map_negative_to_zero(from_travel_time_int + 1);  // left-open interval

  return from_travel_time <= to_travel_time;
}

bool parse_departure_delay_interval(
    std::string const& from_delay_str, std::string const& to_delay_str,
    unsigned int const max_expected_departure_delay, unsigned int& from_delay,
    unsigned int& to_delay) {
  int to_delay_int;
  if (!parse_integer(to_delay_str, to_delay_int)) {
    to_delay = max_expected_departure_delay;
  } else {
    if (to_delay_int <= 0) {
      return false;
    } else if ((unsigned int)to_delay_int <= max_expected_departure_delay) {
      to_delay = to_delay_int - 1;  // right-open interval
    } else /* to_delay_int > max_expected_departure_delay */ {
      to_delay = max_expected_departure_delay;
    }
  }

  int from_delay_int;
  if (!parse_integer(from_delay_str, from_delay_int)) {
    return false;
  }
  from_delay = map_negative_to_zero(from_delay_int);

  return from_delay <= to_delay;
}

void parse_mappings(std::vector<mapping_csv> const& mapping_entries,
                    unsigned int const max_expected_travel_time,
                    unsigned int const max_expected_departure_delay,
                    std::vector<mapping_int>& parsed_mappings) {
  for (auto m : mapping_entries) {
    assert(std::get<mapping_pos::m_distribution_id>(m) >= 0);
    unsigned int from_travel_time, to_travel_time, from_delay, to_delay;
    if (parse_travel_time_interval(std::get<mapping_pos::m_from_travel_time>(m),
                                   std::get<mapping_pos::m_to_travel_time>(m),
                                   max_expected_travel_time, from_travel_time,
                                   to_travel_time) &&
        parse_departure_delay_interval(std::get<mapping_pos::m_from_delay>(m),
                                       std::get<mapping_pos::m_to_delay>(m),
                                       max_expected_departure_delay, from_delay,
                                       to_delay)) {
      parsed_mappings.emplace_back(
          std::get<mapping_pos::m_distribution_id>(m),
          std::get<mapping_pos::m_distribution_class>(m), from_travel_time,
          to_travel_time, from_delay, to_delay);
    }
  }
}

void resolve_mappings(std::vector<mapping_int> const& integer_mappings,
                      std::vector<resolved_mapping>& resolved_mappings) {
  for (auto const& im : integer_mappings) {
    for (unsigned int t = std::get<mapping_pos::m_from_travel_time>(im);
         t <= std::get<mapping_pos::m_to_travel_time>(im); t++) {
      for (unsigned int d = std::get<mapping_pos::m_from_delay>(im);
           d <= std::get<mapping_pos::m_to_delay>(im); d++) {
        resolved_mappings.emplace_back(
            std::get<mapping_pos::m_distribution_class>(im), t, d,
            std::get<mapping_pos::m_distribution_id>(im));
      }
    }
  }
}

inline bool mapping_is_smaller(resolved_mapping const& a,
                               resolved_mapping const& b) {
  if (std::get<resolved_mapping_pos::rm_class>(a) <
      std::get<resolved_mapping_pos::rm_class>(b)) {
    return true;
  } else if (std::get<resolved_mapping_pos::rm_class>(a) ==
             std::get<resolved_mapping_pos::rm_class>(b)) {
    if (std::get<resolved_mapping_pos::rm_travel_time>(a) <
        std::get<resolved_mapping_pos::rm_travel_time>(b)) {
      return true;
    } else if (std::get<resolved_mapping_pos::rm_travel_time>(a) ==
                   std::get<resolved_mapping_pos::rm_travel_time>(b) &&
               std::get<resolved_mapping_pos::rm_delay>(a) <
                   std::get<resolved_mapping_pos::rm_delay>(b)) {
      return true;
    }
  }
  return false;
}

void check_mapping(resolved_mapping const& current,
                   resolved_mapping const& next) {
  if (std::get<resolved_mapping_pos::rm_class>(current) ==
      std::get<resolved_mapping_pos::rm_class>(next)) {
    if (std::get<resolved_mapping_pos::rm_travel_time>(current) ==
        std::get<resolved_mapping_pos::rm_travel_time>(next)) {
      assert(std::get<resolved_mapping_pos::rm_delay>(current) + 1 ==
             std::get<resolved_mapping_pos::rm_delay>(next));
    } else {
      assert(std::get<resolved_mapping_pos::rm_travel_time>(current) + 1 ==
             std::get<resolved_mapping_pos::rm_travel_time>(next));
    }
  }
}

/*********************** functions to read the files **********************/

void load_distributions_classes(
    std::string const filepath,
    std::map<std::string, std::string>& family_to_distribution_class) {
  std::vector<classes_csv> classes_entries;
  parser::read_file<classes_csv, ';'>(filepath.c_str(), classes_entries,
                                      classes_columns);

  for (auto const& entry : classes_entries) {
    if (!std::get<classes_pos::c_distribution_class>(entry).empty()) {
      family_to_distribution_class[std::get<classes_pos::c_family>(entry)] =
          std::get<classes_pos::c_distribution_class>(entry);
    }
  }
}

void load_distributions(
    std::string const filepath,
    std::vector<std::pair<unsigned int, probability_distribution> >&
        probability_distributions) {
  std::vector<distributions_csv> distributions_entries;
  parser::read_file<distributions_csv, ';'>(
      filepath.c_str(), distributions_entries, distributions_columns);
  std::sort(distributions_entries.begin(), distributions_entries.end(),
            distribution_is_smaller<distributions_csv,
                                    distributions_pos::d_distribution_id,
                                    distributions_pos::d_delay_minute>);
  parse_distributions<distributions_csv, unsigned int,
                      distributions_pos::d_distribution_id,
                      distributions_pos::d_delay_minute,
                      distributions_pos::d_delay_probability>(
      distributions_entries, probability_distributions);
}

void load_distribution_mappings(
    std::string const filepath, unsigned int const max_expected_travel_time,
    unsigned int const max_expected_departure_delay,
    std::vector<resolved_mapping>& resolved_mappings) {
  std::vector<mapping_csv> mapping_entries;
  parser::read_file<mapping_csv, ';'>(filepath.c_str(), mapping_entries,
                                      mapping_columns);

  std::vector<mapping_int> parsed_mappings;
  parse_mappings(mapping_entries, max_expected_travel_time,
                 max_expected_departure_delay, parsed_mappings);

  resolve_mappings(parsed_mappings, resolved_mappings);

  std::sort(resolved_mappings.begin(), resolved_mappings.end(),
            mapping_is_smaller);

  for (unsigned int i = 0; i + 1 < resolved_mappings.size(); i++) {
    check_mapping(resolved_mappings[i], resolved_mappings[i + 1]);
  }
}

void load_start_distributions(std::string const filepath,
                              std::map<std::string, probability_distribution>&
                                  class_to_probability_distributions) {
  std::vector<start_distributions_csv> distributions_entries;
  parser::read_file<start_distributions_csv, ';'>(
      filepath.c_str(), distributions_entries, start_distributions_columns);
  std::sort(
      distributions_entries.begin(), distributions_entries.end(),
      distribution_is_smaller<start_distributions_csv,
                              start_distributions_pos::s_distribution_class,
                              start_distributions_pos::s_delay_minute>);
  std::vector<std::pair<std::string, probability_distribution> >
      probability_distributions;
  parse_distributions<start_distributions_csv, std::string,
                      start_distributions_pos::s_distribution_class,
                      start_distributions_pos::s_delay_minute,
                      start_distributions_pos::s_delay_probability>(
      distributions_entries, probability_distributions);

  for (auto it : probability_distributions) {
    assert(class_to_probability_distributions.find(it.first) ==
           class_to_probability_distributions.end());

    /* We do not expect departures earlier than scheduled.
     * If the distribution has delay minutes smaller than 0,
     * add their probabilities to delay minute 0 */
    if (it.second.first_minute() >= 0) {
      class_to_probability_distributions[it.first] = it.second;
    } else {
      std::vector<probability> probabilities;
      probabilities.push_back(it.second.probability_smaller_equal(0));
      for (int d = 1; d <= it.second.last_minute(); d++) {
        probabilities.push_back(it.second.probability_equal(d));
      }
      probability_distribution pd;
      pd.init(probabilities, 0);
      class_to_probability_distributions[it.first] = pd;
    }
  }
}

}  // namespace detail
}  // namespace distributions_loader
}  // namespace reliability
}  // namespace motis
