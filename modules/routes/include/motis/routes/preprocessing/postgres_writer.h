#pragma once

#include <fstream>
#include <string>
#include <vector>

namespace motis {
namespace routes {

class postgres_writer {
public:
  explicit postgres_writer(std::string file_name_)
      : column_size_(0),
        table_open_(false),
        file_name_(std::move(file_name_)){};

  void open() { sql_file_.open(file_name_ + ".sql"); }

  void add_table(std::string const& table_name, std::string const& object,
                 std::string const& simple_object, int column_size) {
    if (sql_file_.is_open() && !table_open_) {
      sql_file_ << "DROP TABLE IF EXISTS " << table_name << ";\n\n";
      sql_file_ << "CREATE TABLE " << table_name << " " << object << ";\n\n";
      sql_file_ << "COPY " << table_name << " " << simple_object
                << " FROM STDIN;\n";
      table_open_ = true;
      column_size_ = column_size;
    }
  }

  void add_entry(std::vector<std::string> const& columns) {
    if (!sql_file_.is_open() || !table_open_ ||
        columns.size() != column_size_) {
      return;
    }
    for (uint32_t i = 0; i < columns.size(); i++) {
      if (i != columns.size() - 1) {
        sql_file_ << columns[i] << "\t";
      } else {
        sql_file_ << columns[i] << "\n";
      }
    }
  }

  void end_table() {
    if (table_open_) {
      sql_file_ << "\\.";
      table_open_ = false;
    }
  }

  std::string make_point(double lng, double lat) {
    return "ST_SetSRID(ST_MakePoint(" + std::to_string(lng) + "," +
           std::to_string(lat) + "), 4326)";
  }

  std::string make_line(std::vector<double> const& locations) {
    std::string line = "ST_GeomFromText('LINESTRING(";
    unsigned i = 0;
    while (i < locations.size()) {
      if (i != locations.size() - 2) {
        line += std::to_string(locations[i]) + " " +
                std::to_string(locations[i + 1]) + ", ";
      } else {
        line += std::to_string(locations[i]) + " " +
                std::to_string(locations[i + 1]);
      }
      i += 2;
    }
    line += ")',4269)";
    return line;
  }

  void close() {
    end_table();
    sql_file_.close();
  }

  unsigned int column_size_;
  bool table_open_;
  std::ofstream sql_file_;
  std::string file_name_;
};

}  // namespace routes
}  // namespace motis
