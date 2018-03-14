#pragma once

#include <limits.h>
#include <vector>
#include "parser/cstr.h"

struct field {
  static const int MAX_SIZE = std::numeric_limits<int>::max();
  int from_, size_;
};

struct attribute {
  field code_;
  field text_mul_spaces_;
  field text_normal_;
};
struct bitfield {
  field index_;
  field value_;
};
struct categories {
  field code_;
  field output_rule_;
  field name_;
};

struct directions {
  field eva_;
  field text_;
};

struct merge_split_rules {
  unsigned line_length_;
  field bitfield_;
  field key1_nr_;
  field key1_admin_;
  field key2_nr_;
  field key2_admin_;
  field eva_begin_;
  field eva_end_;
};

struct meta_stations {
  field eva_;
};
struct footpaths {
  field from_;
  field to_;
  field duration_;
};
struct meta_data {
  meta_stations meta_stations_;
  footpaths footpaths_;
};
struct names {
  field eva_;
  field name_;
};
struct coords {
  field eva_;
  field lng_;
  field lat_;
};

struct stations {
  names names_;
  coords coords_;
};

struct through_services {
  field bitfield_;
  field key1_nr_;
  field key1_admin_;
  field key2_nr_;
  field key2_admin_;
  field eva_;
};

struct timezones {
  field type1_eva_;
  field type1_first_valid_eva_;

  field type2_eva_ = {0, 7};
  field type2_dst_to_midnight_;

  field type3_dst_to_midnight1_;
  field type3_bitfield_idx1_;
  field type3_bitfield_idx2_;
  field type3_dst_to_midnight2_;
  field type3_dst_to_midnight3_;
};
struct track {
  field prov_nr_;
};
struct track_rules {
  field eva_num_;
  field train_num_;
  field train_admin_;
  field track_name_;
  field time_;
  field bitfield_;
};

struct files {
  static constexpr char const* ENCODING = "ISO8859-1";

  static constexpr char const* SCHEDULE_DATA = "fahrten";
  static constexpr char const* CORE_DATA = "stamm";

  std::vector<std::vector<std::string>> required_files_;
};

struct hrd_5_00_8_files : files {
  static constexpr char const* ATTRIBUTES_FILE_OLD = "attributd_int_int.101";
  static constexpr char const* ATTRIBUTES_FILE_NEW = "attributd_int.101";
  static constexpr char const* STATIONS_FILE = "bahnhof.101";
  static constexpr char const* COORDINATES_FILE = "dbkoord_geo.101";
  static constexpr char const* BITFIELDS_FILE = "bitfield.101";
  static constexpr char const* TRACKS_FILE = "gleise.101";
  static constexpr char const* INFOTEXT_FILE = "infotext.101";
  static constexpr char const* BASIC_DATA_FILE = "eckdaten.101";
  static constexpr char const* CATEGORIES_FILE = "zugart_int.101";
  static constexpr char const* DIRECTIONS_FILE = "richtung.101";
  static constexpr char const* PROVIDERS_FILE = "unternehmen_ris.101";
  static constexpr char const* THROUGH_SERVICES_FILE = "durchbi.101";
  static constexpr char const* MERGE_SPLIT_SERVICES_FILE = "vereinig_vt.101";
  static constexpr char const* TIMEZONES_FILE = "zeitvs.101";
  static constexpr char const* FOOTPATHS_REG_FILE = "metabhf.101";
  static constexpr char const* FOOTPATHS_EXT_FILE = "metabhf_zusatz.101";

  hrd_5_00_8_files() {
    required_files_ = {{ATTRIBUTES_FILE_OLD, ATTRIBUTES_FILE_NEW},
                       {STATIONS_FILE},
                       {COORDINATES_FILE},
                       {BITFIELDS_FILE},
                       {TRACKS_FILE},
                       {INFOTEXT_FILE},
                       {BASIC_DATA_FILE},
                       {CATEGORIES_FILE},
                       {DIRECTIONS_FILE},
                       {PROVIDERS_FILE},
                       {THROUGH_SERVICES_FILE},
                       {MERGE_SPLIT_SERVICES_FILE},
                       {TIMEZONES_FILE},
                       {FOOTPATHS_REG_FILE},
                       {FOOTPATHS_EXT_FILE}};
  }
};

struct hrd_5_20_26_files : files {
  static constexpr char const* ATTRIBUTES_FILE = "attributd.txt";
  static constexpr char const* STATIONS_FILE = "bahnhof.txt";
  static constexpr char const* COORDINATES_FILE = "bfkoord.txt";
  static constexpr char const* BITFIELDS_FILE = "bitfield.txt";
  static constexpr char const* TRACKS_FILE = "gleise.txt";
  static constexpr char const* INFOTEXT_FILE = "infotext.txt";
  static constexpr char const* BASIC_DATA_FILE = "eckdaten.txt";
  static constexpr char const* CATEGORIES_FILE = "zugart.txt";
  static constexpr char const* DIRECTIONS_FILE = "richtung.txt";
  static constexpr char const* PROVIDERS_FILE = "unternehmen_ris.txt";
  static constexpr char const* THROUGH_SERVICES_FILE = "durchbi.txt";
  static constexpr char const* MERGE_SPLIT_SERVICES_FILE = "vereinig_vt.txt";
  static constexpr char const* TIMEZONES_FILE = "zeitvs.txt";
  static constexpr char const* FOOTPATHS_REG_FILE = "metabhf.txt";

  hrd_5_20_26_files() {
    required_files_ = {
        {ATTRIBUTES_FILE}, {STATIONS_FILE},         {COORDINATES_FILE},
        {BITFIELDS_FILE},  {TRACKS_FILE},           {INFOTEXT_FILE},
        {BASIC_DATA_FILE}, {CATEGORIES_FILE},       {DIRECTIONS_FILE},
        {PROVIDERS_FILE},  {THROUGH_SERVICES_FILE}, {MERGE_SPLIT_SERVICES_FILE},
        {TIMEZONES_FILE},  {FOOTPATHS_REG_FILE},    {}};
  }
};
struct range_parse_information {
  int from_eva_or_idx_start_;
  int to_eva_or_idx_start_;
  int from_hhmm_or_idx_start_;
  int to_hhmm_or_idx_start_;
};

struct config {
  attribute att_;
  bitfield bf_;
  categories cat_;
  directions dir_;
  merge_split_rules merge_spl_;
  meta_data meta_;
  stations st_;
  through_services th_s_;
  timezones tz_;
  track track_;
  track_rules track_rul_;

  range_parse_information attribute_parse_info_;
  range_parse_information line_parse_info_;
  range_parse_information category_parse_info_;
  range_parse_information traffic_days_parse_info_;
  range_parse_information direction_parse_info_;

  files files_;
  parser::cstr version_;

  parser::cstr parse_field(parser::cstr s, field f) const {
    if (f.size_ != f.MAX_SIZE) {
      return s.substr(f.from_, parser::size(f.size_));
    } else {
      return s.substr(f.from_, s.len - 1);
    }
  }
};

struct hrd_5_00_8 : config {
  hrd_5_00_8() {
    att_ = attribute{{0, 2}, {21, field::MAX_SIZE}, {12, field::MAX_SIZE}};
    bf_ = bitfield{{0, 6}, {7, field::MAX_SIZE}};
    cat_ = categories{{0, 3}, {9, 2}, {12, 8}};
    dir_ = directions{{0, 7}, {8, field::MAX_SIZE}};
    merge_spl_ = merge_split_rules{53,      {47, 6}, {18, 5}, {25, 6},
                                   {33, 5}, {48, 6}, {0, 7},  {9, 7}};
    meta_ = meta_data{{{0, 7}}, {{0, 7}, {8, 7}, {16, 3}}};
    st_ =
        stations{{{0, 2}, {12, field::MAX_SIZE}}, {{0, 7}, {8, 10}, {19, 10}}};
    th_s_ =
        through_services{{34, 6}, {0, 5}, {6, 6}, {21, 5}, {27, 6}, {13, 7}};
    tz_ = timezones{{0, 7},  {8, 7},  {0, 7},  {8, 5}, {14, 5},
                    {20, 8}, {34, 8}, {29, 4}, {43, 4}};
    track_ = track{{0, 5}};
    track_rul_ =
        track_rules{{0, 7}, {8, 5}, {14, 6}, {21, 8}, {30, 4}, {35, 6}};

    attribute_parse_info_ = {6, 14, 29, 36};
    line_parse_info_ = {9, 17, 25, 32};
    category_parse_info_ = {7, 15, 23, 30};
    traffic_days_parse_info_ = {6, 14, 29, 36};
    direction_parse_info_ = {13, 21, 29, 36};

    files_ = hrd_5_00_8_files();
    version_ = "hrd_5_00_8";
  }
};

struct hrd_5_20_26 : config {
  hrd_5_20_26() {
    att_ = attribute{{0, 2}, {21, field::MAX_SIZE}, {12, field::MAX_SIZE}};
    bf_ = bitfield{{0, 6}, {7, field::MAX_SIZE}};
    cat_ = categories{{0, 3}, {9, 2}, {12, 8}};
    dir_ = directions{{0, 7}, {8, field::MAX_SIZE}};
    merge_spl_ = merge_split_rules{50,      {44, 6}, {18, 6}, {23, 6},
                                   {30, 6}, {47, 6}, {0, 7},  {8, 7}};
    meta_ = meta_data{{{0, 7}}, {{0, 7}, {8, 7}, {16, 3}}};
    st_ =
        stations{{{0, 2}, {12, field::MAX_SIZE}}, {{0, 7}, {8, 10}, {19, 10}}};
    th_s_ =
        through_services{{34, 6}, {0, 5}, {6, 6}, {21, 5}, {27, 6}, {13, 7}};
    tz_ = timezones{{0, 7},  {8, 7},  {0, 7},  {8, 5}, {14, 5},
                    {20, 8}, {34, 8}, {29, 4}, {43, 4}};
    track_ = track{{0, 5}};
    track_rul_ =
        track_rules{{0, 7}, {8, 5}, {14, 6}, {21, 8}, {30, 4}, {35, 6}};

    attribute_parse_info_ = {6, 14, 29, 36};
    line_parse_info_ = {12, 20, 28, 35};
    category_parse_info_ = {7, 15, 23, 30};
    traffic_days_parse_info_ = {6, 14, 29, 36};
    direction_parse_info_ = {13, 21, 29, 36};

    files_ = hrd_5_20_26_files();
    version_ = "hrd_5_20_26";
  }
};

const hrd_5_00_8 hrd_5_00_8_;
const hrd_5_20_26 hrd_5_20_26_;
