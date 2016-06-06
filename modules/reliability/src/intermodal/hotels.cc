#include "motis/reliability/intermodal/hotels.h"

#include "parser/csv.h"

namespace motis {
namespace reliability {
namespace intermodal {

void parse_hotels(std::string const file_path, std::vector<hotel>& hotels,
                  uint16_t const earliest_checkout,
                  uint16_t const min_stay_duration, uint16_t const price) {
  using hotels_csv = std::tuple<std::string, std::string, uint16_t>;
  parser::column_mapping<hotels_csv> const hotels_columns = {
      {"station_eva", "hotel_name", "hotel_rating"}};
  enum hotels_pos { h_station, h_name, h_rating };

  std::vector<hotels_csv> hotels_entries;
  parser::read_file<hotels_csv, ','>(file_path.c_str(), hotels_entries,
                                     hotels_columns);
  for (auto const& entry : hotels_entries) {
    hotels.emplace_back(std::get<hotels_pos::h_station>(entry),
                        earliest_checkout, min_stay_duration, price);
  }
}

}  // namespace intermodal
}  // namespace reliability
}  // namespace motis
