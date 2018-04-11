#pragma once

#include <climits>
#include <vector>

#include "parser/cstr.h"

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
    parser::field code_;
    parser::field text_mul_spaces_;
    parser::field text_normal_;
  } att_;
  struct {
    parser::field index_;
    parser::field value_;
  } bf_;
  struct {
    parser::field code_;
    parser::field output_rule_;
    parser::field name_;
  } cat_;
  struct {
    parser::field eva_;
    parser::field text_;
  } dir_;
  struct {
    unsigned line_length_;
    parser::field bitfield_;
    parser::field key1_nr_;
    parser::field key1_admin_;
    parser::field key2_nr_;
    parser::field key2_admin_;
    parser::field eva_begin_;
    parser::field eva_end_;
  } merge_spl_;
  struct {
    struct {
      parser::field eva_;
    } meta_stations_;
    struct {
      parser::field from_;
      parser::field to_;
      parser::field duration_;
    } footpaths_;
  } meta_;
  struct {
    struct {
      parser::field eva_;
      parser::field name_;
    } names_;
    struct {
      parser::field eva_;
      parser::field lng_;
      parser::field lat_;
    } coords_;
  } st_;
  struct {
    parser::field bitfield_;
    parser::field key1_nr_;
    parser::field key1_admin_;
    parser::field key2_nr_;
    parser::field key2_admin_;
    parser::field eva_;
  } th_s_;
  struct {
    parser::field type1_eva_;
    parser::field type1_first_valid_eva_;

    parser::field type2_eva_ = {0, 7};
    parser::field type2_dst_to_midnight_;

    parser::field type3_dst_to_midnight1_;
    parser::field type3_bitfield_idx1_;
    parser::field type3_bitfield_idx2_;
    parser::field type3_dst_to_midnight2_;
    parser::field type3_dst_to_midnight3_;
  } tz_;
  struct {
    parser::field prov_nr_;
  } track_;
  struct {
    parser::field eva_num_;
    parser::field train_num_;
    parser::field train_admin_;
    parser::field track_name_;
    parser::field time_;
    parser::field bitfield_;
  } track_rul_;
  struct {
    parser::field att_eva_;
    parser::field att_code_;
    parser::field cat_;
    parser::field line_;
    parser::field traff_days_;
    parser::field dir_;
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
};

const config hrd_5_00_8 = {
    .att_ = {.code_ = {0, 2},
             .text_mul_spaces_ = {21, parser::field::MAX_SIZE},
             .text_normal_ = {12, parser::field::MAX_SIZE}},
    .bf_ = {.index_ = {0, 6}, .value_ = {7, parser::field::MAX_SIZE}},
    .cat_ = {.code_ = {0, 3}, .output_rule_ = {9, 2}, .name_ = {12, 8}},
    .dir_ = {.eva_ = {0, 7}, .text_ = {8, parser::field::MAX_SIZE}},
    .merge_spl_ =
        {
            .line_length_ = 53,
            .bitfield_ = {47, 6},
            .key1_nr_ = {18, 5},
            .key1_admin_ = {25, 6},
            .key2_nr_ = {33, 5},
            .key2_admin_ = {40, 6},
            .eva_begin_ = {0, 7},
            .eva_end_ = {9, 7},
        },
    .meta_ = {.meta_stations_ = {.eva_ = {0, 7}},
              .footpaths_ = {.from_ = {0, 7},
                             .to_ = {8, 7},
                             .duration_ = {16, 3}}},
    .st_ =
        {
            .names_ = {.eva_ = {0, 7}, .name_ = {12, parser::field::MAX_SIZE}},
            .coords_ = {.eva_ = {0, 7}, .lng_ = {8, 10}, .lat_ = {19, 10}},
        },
    .th_s_ = {.bitfield_ = {34, 6},
              .key1_nr_ = {0, 5},
              .key1_admin_ = {6, 6},
              .key2_nr_ = {21, 5},
              .key2_admin_ = {27, 6},
              .eva_ = {13, 7}},
    .tz_ = {.type1_eva_ = {0, 7},
            .type1_first_valid_eva_ = {8, 7},

            .type2_eva_ = {0, 7},
            .type2_dst_to_midnight_ = {8, 5},

            .type3_dst_to_midnight1_ = {14, 5},
            .type3_bitfield_idx1_ = {20, 8},
            .type3_bitfield_idx2_ = {34, 8},
            .type3_dst_to_midnight2_ = {29, 4},
            .type3_dst_to_midnight3_ = {43, 4}

    },
    .track_ = {.prov_nr_ = {0, 5}},
    .track_rul_ = {.eva_num_ = {0, 7},
                   .train_num_ = {8, 5},
                   .train_admin_ = {14, 6},
                   .track_name_ = {21, 8},
                   .time_ = {30, 4},
                   .bitfield_ = {35, 6}},

    .s_info_ = {.att_eva_ = {22, 6},
                .att_code_ = {3, 2},
                .cat_ = {3, 3},
                .line_ = {3, 5},
                .traff_days_ = {22, 6},
                .dir_ = {5, 7}},
    .attribute_parse_info_ = {.from_eva_or_idx_start_ = 6,
                              .to_eva_or_idx_start_ = 14,
                              .from_hhmm_or_idx_start_ = 29,
                              .to_hhmm_or_idx_start_ = 36},
    .line_parse_info_ = {.from_eva_or_idx_start_ = 9,
                         .to_eva_or_idx_start_ = 17,
                         .from_hhmm_or_idx_start_ = 25,
                         .to_hhmm_or_idx_start_ = 32},
    .category_parse_info_ = {.from_eva_or_idx_start_ = 7,
                             .to_eva_or_idx_start_ = 15,
                             .from_hhmm_or_idx_start_ = 23,
                             .to_hhmm_or_idx_start_ = 30},
    .traffic_days_parse_info_ = {.from_eva_or_idx_start_ = 6,
                                 .to_eva_or_idx_start_ = 14,
                                 .from_hhmm_or_idx_start_ = 29,
                                 .to_hhmm_or_idx_start_ = 36},
    .direction_parse_info_ = {.from_eva_or_idx_start_ = 13,
                              .to_eva_or_idx_start_ = 21,
                              .from_hhmm_or_idx_start_ = 29,
                              .to_hhmm_or_idx_start_ = 36},
    .files_ = files::hrd_5_00_8_files(),
    .version_ = "hrd_5_00_8"};

const config hrd_5_20_26 = {
    .att_ = {.code_ = {0, 2},
             .text_mul_spaces_ = {21, parser::field::MAX_SIZE},
             .text_normal_ = {12, parser::field::MAX_SIZE}},
    .bf_ = {.index_ = {0, 6}, .value_ = {7, parser::field::MAX_SIZE}},
    .cat_ = {.code_ = {0, 3}, .output_rule_ = {9, 2}, .name_ = {12, 8}},
    .dir_ = {.eva_ = {0, 7}, .text_ = {8, parser::field::MAX_SIZE}},
    .merge_spl_ =
        {
            .line_length_ = 50,
            .bitfield_ = {44, 6},
            .key1_nr_ = {18, 6},
            .key1_admin_ = {23, 6},
            .key2_nr_ = {30, 6},
            .key2_admin_ = {37, 6},
            .eva_begin_ = {0, 7},
            .eva_end_ = {8, 7},
        },
    .meta_ = {.meta_stations_ = {.eva_ = {0, 7}},
              .footpaths_ = {.from_ = {0, 7},
                             .to_ = {8, 7},
                             .duration_ = {16, 3}}},
    .st_ =
        {
            .names_ = {.eva_ = {0, 7}, .name_ = {12, parser::field::MAX_SIZE}},
            .coords_ = {.eva_ = {0, 7}, .lng_ = {8, 10}, .lat_ = {19, 10}},
        },
    .th_s_ = {.bitfield_ = {34, 6},
              .key1_nr_ = {0, 5},
              .key1_admin_ = {6, 6},
              .key2_nr_ = {21, 5},
              .key2_admin_ = {27, 6},
              .eva_ = {13, 7}},
    .tz_ = {.type1_eva_ = {0, 7},
            .type1_first_valid_eva_ = {8, 7},

            .type2_eva_ = {0, 7},
            .type2_dst_to_midnight_ = {8, 5},

            .type3_dst_to_midnight1_ = {14, 5},
            .type3_bitfield_idx1_ = {20, 8},
            .type3_bitfield_idx2_ = {34, 8},
            .type3_dst_to_midnight2_ = {29, 4},
            .type3_dst_to_midnight3_ = {43, 4}

    },
    .track_ = {.prov_nr_ = {0, 5}},
    .track_rul_ = {.eva_num_ = {0, 7},
                   .train_num_ = {8, 5},
                   .train_admin_ = {14, 6},
                   .track_name_ = {21, 8},
                   .time_ = {30, 4},
                   .bitfield_ = {35, 6}},

    .s_info_ = {.att_eva_ = {22, 6},
                .att_code_ = {3, 2},
                .cat_ = {3, 3},
                .line_ = {3, 8},
                .traff_days_ = {22, 6},
                .dir_ = {5, 7}},
    .attribute_parse_info_ = {.from_eva_or_idx_start_ = 6,
                              .to_eva_or_idx_start_ = 14,
                              .from_hhmm_or_idx_start_ = 29,
                              .to_hhmm_or_idx_start_ = 36},
    .line_parse_info_ = {.from_eva_or_idx_start_ = 12,
                         .to_eva_or_idx_start_ = 20,
                         .from_hhmm_or_idx_start_ = 28,
                         .to_hhmm_or_idx_start_ = 35},
    .category_parse_info_ = {.from_eva_or_idx_start_ = 7,
                             .to_eva_or_idx_start_ = 15,
                             .from_hhmm_or_idx_start_ = 23,
                             .to_hhmm_or_idx_start_ = 30},
    .traffic_days_parse_info_ = {.from_eva_or_idx_start_ = 6,
                                 .to_eva_or_idx_start_ = 14,
                                 .from_hhmm_or_idx_start_ = 29,
                                 .to_hhmm_or_idx_start_ = 36},
    .direction_parse_info_ = {.from_eva_or_idx_start_ = 13,
                              .to_eva_or_idx_start_ = 21,
                              .from_hhmm_or_idx_start_ = 29,
                              .to_hhmm_or_idx_start_ = 36},
    .files_ = files::hrd_5_20_26_files(),
    .version_ = "hrd_5_20_26"};
const std::vector<config> configs = {hrd_5_00_8, hrd_5_20_26};
