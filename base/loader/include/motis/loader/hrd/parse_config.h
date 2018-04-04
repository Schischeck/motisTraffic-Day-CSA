#pragma once

#include <climits>
#include <vector>

#include "parser/cstr.h"
struct field {
  int from_, size_;
  static const int MAX_SIZE = std::numeric_limits<int>::max();
};

struct range_parse_information {
  int from_eva_or_idx_start_;
  int to_eva_or_idx_start_;
  int from_hhmm_or_idx_start_;
  int to_hhmm_or_idx_start_;
};

enum filename_key {
  ATTRIBUTES,
  STATIONS,
  COORDINATES,
  BITFIELDS,
  TRACKS,
  INFOTEXT,
  BASIC_DATA,
  CATEGORIES,
  DIRECTIONS,
  PROVIDERS,
  THROUGH_SERVICES,
  MERGE_SPLIT_SERVICES,
  TIMEZONES,
  FOOTPATHS,
  FOOTPATHS_EXT
};

struct files {
  static constexpr auto const ENCODING = "ISO8859-1";

  static constexpr auto const SCHEDULE_DATA = "fahrten";
  static constexpr auto const CORE_DATA = "stamm";

  std::vector<std::vector<std::string>> required_files_;
  static files hrd_5_00_8_files() {
    files f;
    auto const ATTRIBUTES_FILE_OLD = "attributd_int_int.101";
    auto const ATTRIBUTES_FILE_NEW = "attributd_int.101";
    auto const STATIONS_FILE = "bahnhof.101";
    auto const COORDINATES_FILE = "dbkoord_geo.101";
    auto const BITFIELDS_FILE = "bitfield.101";
    auto const TRACKS_FILE = "gleise.101";
    auto const INFOTEXT_FILE = "infotext.101";
    auto const BASIC_DATA_FILE = "eckdaten.101";
    auto const CATEGORIES_FILE = "zugart_int.101";
    auto const DIRECTIONS_FILE = "richtung.101";
    auto const PROVIDERS_FILE = "unternehmen_ris.101";
    auto const THROUGH_SERVICES_FILE = "durchbi.101";
    auto const MERGE_SPLIT_SERVICES_FILE = "vereinig_vt.101";
    auto const TIMEZONES_FILE = "zeitvs.101";
    auto const FOOTPATHS_REG_FILE = "metabhf.101";
    auto const FOOTPATHS_EXT_FILE = "metabhf_zusatz.101";
    f.required_files_ = {{ATTRIBUTES_FILE_OLD, ATTRIBUTES_FILE_NEW},
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
    return f;
  }

  static files hrd_5_20_26_files() {
    files f;
    auto const ATTRIBUTES_FILE = "attributd.txt";
    auto const STATIONS_FILE = "bahnhof.txt";
    auto const COORDINATES_FILE = "bfkoord.txt";
    auto const BITFIELDS_FILE = "bitfield.txt";
    auto const TRACKS_FILE = "gleise.txt";
    auto const INFOTEXT_FILE = "infotext.txt";
    auto const BASIC_DATA_FILE = "eckdaten.txt";
    auto const CATEGORIES_FILE = "zugart.txt";
    auto const DIRECTIONS_FILE = "richtung.txt";
    auto const PROVIDERS_FILE = "unternehmen_ris.txt";
    auto const THROUGH_SERVICES_FILE = "durchbi.txt";
    auto const MERGE_SPLIT_SERVICES_FILE = "vereinig_vt.txt";
    auto const TIMEZONES_FILE = "zeitvs.txt";
    auto const FOOTPATHS_REG_FILE = "metabhf.txt";
    f.required_files_ = {
        {ATTRIBUTES_FILE}, {STATIONS_FILE},         {COORDINATES_FILE},
        {BITFIELDS_FILE},  {TRACKS_FILE},           {INFOTEXT_FILE},
        {BASIC_DATA_FILE}, {CATEGORIES_FILE},       {DIRECTIONS_FILE},
        {PROVIDERS_FILE},  {THROUGH_SERVICES_FILE}, {MERGE_SPLIT_SERVICES_FILE},
        {TIMEZONES_FILE},  {FOOTPATHS_REG_FILE},    {}};
    return f;
  }
};

struct config {
  struct {
    field code_;
    field text_mul_spaces_;
    field text_normal_;
  } att_;
  struct {
    field index_;
    field value_;
  } bf_;
  struct {
    field code_;
    field output_rule_;
    field name_;
  } cat_;
  struct {
    field eva_;
    field text_;
  } dir_;
  struct {
    unsigned line_length_;
    field bitfield_;
    field key1_nr_;
    field key1_admin_;
    field key2_nr_;
    field key2_admin_;
    field eva_begin_;
    field eva_end_;
  } merge_spl_;
  struct {
    struct {
      field eva_;
    } meta_stations_;
    struct {
      field from_;
      field to_;
      field duration_;
    } footpaths_;
  } meta_;
  struct {
    struct {
      field eva_;
      field name_;
    } names_;
    struct {
      field eva_;
      field lng_;
      field lat_;
    } coords_;
  } st_;
  struct {
    field bitfield_;
    field key1_nr_;
    field key1_admin_;
    field key2_nr_;
    field key2_admin_;
    field eva_;
  } th_s_;
  struct {
    field type1_eva_;
    field type1_first_valid_eva_;

    field type2_eva_ = {0, 7};
    field type2_dst_to_midnight_;

    field type3_dst_to_midnight1_;
    field type3_bitfield_idx1_;
    field type3_bitfield_idx2_;
    field type3_dst_to_midnight2_;
    field type3_dst_to_midnight3_;
  } tz_;
  struct {
    field prov_nr_;
  } track_;
  struct {
    field eva_num_;
    field train_num_;
    field train_admin_;
    field track_name_;
    field time_;
    field bitfield_;
  } track_rul_;
  struct {
    field att_eva_;
    field att_code_;
    field cat_;
    field line_;
    field traff_days_;
    field dir_;
  } s_info_;

  range_parse_information attribute_parse_info_;
  range_parse_information line_parse_info_;
  range_parse_information category_parse_info_;
  range_parse_information traffic_days_parse_info_;
  range_parse_information direction_parse_info_;

  files files_;
  parser::cstr version_;

  const char* files(filename_key k, int index = 0) const {
    return files_.required_files_[k][index].c_str();
  }

  parser::cstr substr(parser::cstr line, field f) {
    if (f.size_ == field::MAX_SIZE) {
      return line.substr(f.from_);
    } else {
      return line.substr(f.from_, f.size_);
    }
  }

  static config hrd_5_00_8() {
    config c;
    c.att_ = {{0, 2}, {21, field::MAX_SIZE}, {12, field::MAX_SIZE}};
    c.bf_ = {{0, 6}, {7, field::MAX_SIZE}};
    c.cat_ = {{0, 3}, {9, 2}, {12, 8}};
    c.dir_ = {{0, 7}, {8, field::MAX_SIZE}};
    c.merge_spl_ = {53,      {47, 6}, {18, 5}, {25, 6},
                    {33, 5}, {40, 6}, {0, 7},  {9, 7}};
    c.meta_ = {{{0, 7}}, {{0, 7}, {8, 7}, {16, 3}}};
    c.st_ = {{{0, 7}, {12, field::MAX_SIZE}}, {{0, 7}, {8, 10}, {19, 10}}};
    c.th_s_ = {{34, 6}, {0, 5}, {6, 6}, {21, 5}, {27, 6}, {13, 7}};
    c.tz_ = {{0, 7},  {8, 7},  {0, 7},  {8, 5}, {14, 5},
             {20, 8}, {34, 8}, {29, 4}, {43, 4}};
    c.track_ = {{0, 5}};
    c.track_rul_ = {{0, 7}, {8, 5}, {14, 6}, {21, 8}, {30, 4}, {35, 6}};

    c.attribute_parse_info_ = {6, 14, 29, 36};
    c.line_parse_info_ = {9, 17, 25, 32};
    c.category_parse_info_ = {7, 15, 23, 30};
    c.traffic_days_parse_info_ = {6, 14, 29, 36};
    c.direction_parse_info_ = {13, 21, 29, 36};

    c.s_info_ = {{22, 6}, {3, 2}, {3, 3}, {3, 5}, {22, 6}, {5, 7}};

    c.files_ = files::hrd_5_00_8_files();
    c.version_ = "hrd_5_00_8";
    return c;
  }
  static config hrd_5_20_26() {
    config c;
    c.att_ = {{0, 2}, {21, field::MAX_SIZE}, {12, field::MAX_SIZE}};
    c.bf_ = {{0, 6}, {7, field::MAX_SIZE}};
    c.cat_ = {{0, 3}, {9, 2}, {12, 8}};
    c.dir_ = {{0, 7}, {8, field::MAX_SIZE}};
    c.merge_spl_ = {50,      {44, 6}, {18, 6}, {23, 6},
                    {30, 6}, {37, 6}, {0, 7},  {8, 7}};
    c.meta_ = {{{0, 7}}, {{0, 7}, {8, 7}, {16, 3}}};
    c.st_ = {{{0, 7}, {12, field::MAX_SIZE}}, {{0, 7}, {8, 10}, {19, 10}}};
    c.th_s_ = {{34, 6}, {0, 5}, {6, 6}, {21, 5}, {27, 6}, {13, 7}};
    c.tz_ = {{0, 7},  {8, 7},  {0, 7},  {8, 5}, {14, 5},
             {20, 8}, {34, 8}, {29, 4}, {43, 4}};
    c.track_ = {{0, 5}};
    c.track_rul_ = {{0, 7}, {8, 5}, {14, 6}, {21, 8}, {30, 4}, {35, 6}};

    c.attribute_parse_info_ = {6, 14, 29, 36};
    c.line_parse_info_ = {12, 20, 28, 35};
    c.category_parse_info_ = {7, 15, 23, 30};
    c.traffic_days_parse_info_ = {6, 14, 29, 36};
    c.direction_parse_info_ = {13, 21, 29, 36};

    c.s_info_ = {{22, 6}, {3, 2}, {3, 3}, {3, 8}, {22, 6}, {5, 7}};

    c.files_ = files::hrd_5_20_26_files();
    c.version_ = "hrd_5_20_26";
    return c;
  }
};

const config hrd_5_00_8 = config::hrd_5_00_8();
const config hrd_5_20_26 = config::hrd_5_20_26();
const std::vector<config> configs = {hrd_5_00_8, hrd_5_20_26};
