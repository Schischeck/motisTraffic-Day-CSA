#pragma once

namespace motis {
namespace loader {
namespace hrd {

constexpr char const* ENCODING = "ISO8859-1";

constexpr char const* SCHEDULE_DATA = "fahrten";
constexpr char const* CORE_DATA = "stamm";

constexpr char const* ATTRIBUTES_FILE_OLD = "attributd_int_int.101";
constexpr char const* ATTRIBUTES_FILE_NEW = "attributd_int.101";
constexpr char const* STATIONS_FILE = "bahnhof.101";
constexpr char const* COORDINATES_FILE = "dbkoord_geo.101";
constexpr char const* BITFIELDS_FILE = "bitfield.101";
constexpr char const* PLATFORMS_FILE = "gleise.101";
constexpr char const* INFOTEXT_FILE = "infotext.101";
constexpr char const* BASIC_DATA_FILE = "eckdaten.101";
constexpr char const* CATEGORIES_FILE = "zugart_int.101";
constexpr char const* DIRECTIONS_FILE = "richtung.101";
constexpr char const* PROVIDERS_FILE = "unternehmen_ris.101";
constexpr char const* THROUGH_SERVICES_FILE = "durchbi.101";
constexpr char const* MERGE_SPLIT_SERVICES_FILE = "vereinig_vt.101";

}  // namespace hrd
}  // namespace loader
}  // namespace motis
