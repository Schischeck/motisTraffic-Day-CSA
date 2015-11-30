#include "motis/reliability/tools/hotels.h"

#include "parser/csv.h"

namespace motis {
namespace reliability {
namespace hotels {

std::vector<hotel_info> parse_hotels(std::string const file_path) {
  using hotels_csv = std::tuple<std::string, std::string, unsigned short>;
  parser::column_mapping<hotels_csv> const hotels_columns = {
      {"station_eva", "hotel_name", "hotel_rating"}};
  enum hotels_pos { h_station, h_name, h_rating };

  std::vector<hotels_csv> hotels_entries;
  parser::read_file<hotels_csv, ','>(file_path.c_str(), hotels_entries,
                                     hotels_columns);
  std::vector<hotel_info> hotels;
  for (auto const& entry : hotels_entries) {
    hotels.emplace_back(std::get<hotels_pos::h_station>(entry));
  }
  return hotels;
}

}  // namespace hotels
}  // namespace reliability
}  // namespace motis
