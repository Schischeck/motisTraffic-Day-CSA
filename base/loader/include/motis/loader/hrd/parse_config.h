#pragma once

#include <limits.h>
#include "parser/cstr.h"

namespace parser {

struct field {
  static const int MAX_SIZE = std::numeric_limits<int>::max();
  int from_, size_;
};

cstr parse_field(cstr s, field f) {
  if (f.size_ != f.MAX_SIZE) {
    return s.substr(f.from_, size(f.size_));
  } else {
    return s.substr(f.from_, s.len - 1);
  }
}
struct hrd_5_00_8 {

  struct attribute {
    field code_ = {0, 2};
    field text_mul_spaces_ = {21, field::MAX_SIZE};
    field text_normal_ = {12, field::MAX_SIZE};
  };
  attribute attribute_;
  struct bitfield {
    field index_ = {0, 6};
    field value_ = {7, field::MAX_SIZE};
  };
  bitfield bitfield_;
  struct categories {
    field code_ = {0, 3};
    field output_rule_ = {9, 2};
    field name_ = {12, 8};
  };
  categories categories_;
  struct directions {
    field eva_ = {0, 7};
    field text_ = {8, field::MAX_SIZE};
  };
  directions directions_;

  struct merge_split_rules {
    int line_length_ = 53;
    field bitfield_ = {47, 6};
    field key1_nr_ = {18, 5};
    field key1_admin_ = {25, 6};
    field key2_nr_ = {33, 5};
    field key2_admin_ = {40, 6};
    field eva_begin_ = {0, 7};
    field eva_end_ = {9, 7};
  };
  merge_split_rules merge_split_rules_;

  struct meta_data {
    struct meta_stations {
      field eva_ = {0, 7};
    };

    struct footpaths {
      field from_ = {0, 7};
      field to_ = {8, 7};
      field duration_ = {16, 3};
    };
    meta_stations meta_stations_;
    footpaths footpaths_;
  };
  meta_data meta_data_;

  struct stations {
    struct names {
      field eva_ = {0, 7};
      field name_ = {12, field::MAX_SIZE};
    };
    names names_;
    struct coords {
      field eva_ = {0, 7};
      field lng_ = {8, 10};
      field lat_ = {19, 10};
    };
    coords coords_;
  };
  stations stations_;

  struct through_services {
    field bitfield_ = {34, 6};
    field key1_nr_ = {0, 5};
    field key1_admin_ = {6, 6};
    field key2_nr_ = {21, 5};
    field key2_admin_ = {27, 6};
    field eva_ = {13, 7};
  };
  through_services through_services_;

  struct timezones {
    field type1_eva_ = {0, 7};
    field type1_first_valid_eva_ = {8, 7};

    field type2_eva_ = {0, 7};
    field type2_dst_to_midnight_ = {8, 5};

    field type3_dst_to_midnight1_ = {14, 5};
    field type3_bitfield_idx1_ = {20, 8};
    field type3_bitfield_idx2_ = {34, 8};
    field type3_dst_to_midnight2_ = {29, 4};
    field type3_dst_to_midnight3_ = {43, 4};
  };

  timezones timezones_;

  struct track {
    field prov_nr_ = {0, 5};
  };
  track track_;
  struct track_rules {
    field eva_num_ = {0, 7};
    field train_num_ = {8, 5};
    field train_admin_ = {14, 6};
    field track_name_ = {21, 8};
    field time_ = {30, 4};
    field bitfield_ = {35, 6};
  };
  track_rules track_rules_;

  struct files {
    constexpr char const* ENCODING = "ISO8859-1";

    constexpr char const* SCHEDULE_DATA = "fahrten";
    constexpr char const* CORE_DATA = "stamm";

    constexpr char const* ATTRIBUTES_FILE_OLD = "attributd_int_int.101";
    constexpr char const* ATTRIBUTES_FILE_NEW = "attributd_int.101";
    constexpr char const* STATIONS_FILE = "bahnhof.101";
    constexpr char const* COORDINATES_FILE = "dbkoord_geo.101";
    constexpr char const* BITFIELDS_FILE = "bitfield.101";
    constexpr char const* TRACKS_FILE = "gleise.101";
    constexpr char const* INFOTEXT_FILE = "infotext.101";
    constexpr char const* BASIC_DATA_FILE = "eckdaten.101";
    constexpr char const* CATEGORIES_FILE = "zugart_int.101";
    constexpr char const* DIRECTIONS_FILE = "richtung.101";
    constexpr char const* PROVIDERS_FILE = "unternehmen_ris.101";
    constexpr char const* THROUGH_SERVICES_FILE = "durchbi.101";
    constexpr char const* MERGE_SPLIT_SERVICES_FILE = "vereinig_vt.101";
    constexpr char const* TIMEZONES_FILE = "zeitvs.101";
    constexpr char const* FOOTPATHS_REG_FILE = "metabhf.101";
    constexpr char const* FOOTPATHS_EXT_FILE = "metabhf_zusatz.101";

    std::vector<std::vector<std::string>> const required_files_ = {
        {ATTRIBUTES_FILE_OLD, ATTRIBUTES_FILE_NEW},
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
  };
  files files_;
  cstr version_ = "hrd_5_00_8";
};

struct hrd_5_20_26 {

  struct attribute {
    field code_ = {0, 2};
    field text_mul_spaces_ = {21, field::MAX_SIZE};
    field text_normal_ = {12, field::MAX_SIZE};
  };
  attribute attribute_;
  struct bitfield {
    field index_ = {0, 6};
    field value_ = {7, field::MAX_SIZE};
  };
  bitfield bitfield_;
  struct categories {
    field code_ = {0, 3};
    field output_rule_ = {9, 2};
    field name_ = {12, 8};
  };
  categories categories_;
  struct directions {
    field eva_ = {0, 7};
    field text_ = {8, field::MAX_SIZE};
  };
  directions directions_;

  struct merge_split_rules {
    int line_length_ = 50;
    field bitfield_ = {44, 6};
    field key1_nr_ = {18, 6};
    field key1_admin_ = {23, 6};
    field key2_nr_ = {30, 6};
    field key2_admin_ = {37, 6};
    field eva_begin_ = {0, 7};
    field eva_end_ = {8, 7};
  };
  merge_split_rules merge_split_rules_;

  struct meta_data {
    struct meta_stations {
      field eva_ = {0, 7};
    };

    struct footpaths {
      field from_ = {0, 7};
      field to_ = {8, 7};
      field duration_ = {16, 3};
    };
    meta_stations meta_stations_;
    footpaths footpaths_;
  };
  meta_data meta_data_;

  struct stations {
    struct names {
      field eva_ = {0, 7};
      field name_ = {12, field::MAX_SIZE};
    };
    names names_;
    struct coords {
      field eva_ = {0, 7};
      field lng_ = {8, 10};
      field lat_ = {19, 10};
    };
    coords coords_;
  };
  stations stations_;

  struct through_services {
    field bitfield_ = {34, 6};
    field key1_nr_ = {0, 5};
    field key1_admin_ = {6, 6};
    field key2_nr_ = {21, 5};
    field key2_admin_ = {27, 6};
    field eva_ = {13, 7};
  };
  through_services through_services_;

  struct timezones {
    field type1_eva_ = {0, 7};
    field type1_first_valid_eva_ = {8, 7};

    field type2_eva_ = {0, 7};
    field type2_dst_to_midnight_ = {8, 5};

    field type3_dst_to_midnight1_ = {14, 5};
    field type3_bitfield_idx1_ = {20, 8};
    field type3_bitfield_idx2_ = {34, 8};
    field type3_dst_to_midnight2_ = {29, 4};
    field type3_dst_to_midnight3_ = {43, 4};
  };

  timezones timezones_;

  struct track {
    field prov_nr_ = {0, 5};
  };
  track track_;
  struct track_rules {
    field eva_num_ = {0, 7};
    field train_num_ = {8, 5};
    field train_admin_ = {14, 6};
    field track_name_ = {21, 8};
    field time_ = {30, 4};
    field bitfield_ = {35, 6};
  };
  track_rules track_rules_;

  struct files {
    constexpr char const* ENCODING = "ISO8859-1";

    constexpr char const* SCHEDULE_DATA = "fahrten";
    constexpr char const* CORE_DATA = "stamm";

    constexpr char const* ATTRIBUTES_FILE = "attributd.txt";
    constexpr char const* STATIONS_FILE = "bahnhof.txt";
    constexpr char const* COORDINATES_FILE = "bfkoord.txt";
    constexpr char const* BITFIELDS_FILE = "bitfield.txt";
    constexpr char const* TRACKS_FILE = "gleise.txt";
    constexpr char const* INFOTEXT_FILE = "infotext.txt";
    constexpr char const* BASIC_DATA_FILE = "eckdaten.txt";
    constexpr char const* CATEGORIES_FILE = "zugart.txt";
    constexpr char const* DIRECTIONS_FILE = "richtung.txt";
    constexpr char const* PROVIDERS_FILE = "unternehmen_ris.txt";
    constexpr char const* THROUGH_SERVICES_FILE = "durchbi.txt";
    constexpr char const* MERGE_SPLIT_SERVICES_FILE = "vereinig_vt.txt";
    constexpr char const* TIMEZONES_FILE = "zeitvs.txt";
    constexpr char const* FOOTPATHS_REG_FILE = "metabhf.txt";

    std::vector<std::vector<std::string>> const required_files_ = {
        {ATTRIBUTES_FILE}, {STATIONS_FILE},         {COORDINATES_FILE},
        {BITFIELDS_FILE},  {TRACKS_FILE},           {INFOTEXT_FILE},
        {BASIC_DATA_FILE}, {CATEGORIES_FILE},       {DIRECTIONS_FILE},
        {PROVIDERS_FILE},  {THROUGH_SERVICES_FILE}, {MERGE_SPLIT_SERVICES_FILE},
        {TIMEZONES_FILE},  {FOOTPATHS_REG_FILE},    {}};
  };
  files files_;
  cstr version_ = "hrd_5_20_26";
};

const hrd_5_00_8 hrd_5_00_8_;
const hrd_5_20_26 hrd_5_20_26_;

}  // namespace parser
