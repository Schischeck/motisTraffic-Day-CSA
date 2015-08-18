#pragma once

#include <map>
#include <string>

#include "motis/reliability/probability_distribution.h"

#include "parser/csv.h"

namespace motis {
namespace reliability {

/**
 * Functions to read CSV-files (with ';' separator) containing
 * distributions for travel-times and start events of trains.
 * Note: the files have to be in Unix-format without carriage return
 * and usage of "Umlaut" in the headers is not allowed.
 * Float-values have to use '.' as decimal mark.
 */
namespace distributions_loader {
/* distribution classes */
using classes_csv = std::tuple<std::string, std::string>;
parser::column_mapping<classes_csv> const classes_columns = {
    {"Zuggattung", "Zuggattungsklasse"}};
enum classes { c_family, c_distribution_class };

/* distributions */
using distributions_csv = std::tuple<int, int, probability>;
parser::column_mapping<distributions_csv> const distributions_columns = {
    {"KLASSEN_ID", "verspaetungsDelta", "p_Verspaetungsdelta"}};
enum distributions { d_distribution_id, d_delay_minute, d_delay_probability };

/* distribution mapping */
using mapping_csv = std::tuple<int, parser::cstr, int, int, int, int>;
parser::column_mapping<mapping_csv> const mapping_columns = {
    {"KLASSEN_ID", "ZUGGATTUNGSKLASSE", "VORSCHAUZEITKLASSE_VON",
     "VORSCHAUZEITKLASSE_BIS", "AKTUELLEVERSPAETUNGSLAGE_VON",
     "AKTUELLEVERSPAETUNGSLAGE_BIS"}};
enum mapping {
  m_distribution_id,
  m_distribution_class,
  m_from_travel_time,
  m_to_travel_time,
  m_from_delay,
  m_to_delay
};

/* start distributions */
using start_distributions_csv = std::tuple<parser::cstr, int, probability>;
parser::column_mapping<start_distributions_csv> const
    start_distributions_columns = {
        {"Zuggattungsklasse", "Startverspaetung", "p_startverspaetung"}};
enum start_distributions {
  s_distribution_class,
  s_delay_minute,
  s_delay_probability
};

void load_distributions(std::string const root);

namespace detail {
void load_distributions_classes(
    std::string const filepath,
    std::map<std::string, std::string>& family_to_distribution_class);
void load_distributions(
    std::string const filepath,
    std::vector<probability_distribution>& probability_distributions);
void load_distribution_mappings(std::string const filepath);
void load_start_distributions(std::string const filepath);
}  // namespace detail
}  // namespace distributions_loader

}  // namespace reliability
}  // namespace motis
