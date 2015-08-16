#include <cinttypes>
#include <cstring>

#include "catch/catch.hpp"

#include "parser/cstr.h"

#include "motis/loader/parser_error.h"
#include "motis/loader/parsers/hrd/files.h"
#include "motis/loader/parsers/hrd/attributes_parser.h"
#include "motis/loader/parsers/hrd/bitfields_parser.h"
#include "motis/loader/parsers/hrd/stations_parser.h"
#include "motis/loader/parsers/hrd/platform_rules_parser.h"
#include "motis/loader/parsers/hrd/trains_parser.h"
#include "motis/loader/util.h"
#include "motis/schedule-format/Schedule_generated.h"

using namespace parser;
using namespace flatbuffers;

namespace motis {
namespace loader {
namespace hrd {

cstr trains_file_content = R"(
*Z 02292 80____    01                                     % 02292 80____    01 (001)
*G IC  8000096 8000105                                    % 02292 80____    01 (002)
*A VE 8000096 8000105 002687                              % 02292 80____    01 (003)
*I FZ                        0129939                      % 02292 80____    01 (004)
*A BT 8000096 8000105                                     % 02292 80____    01 (006)
*A FR 8000096 8000105                                     % 02292 80____    01 (007)
*A G  8000096 8000105                                     % 02292 80____    01 (008)
8000096 Stuttgart Hbf                01605                % 02292 80____    01 (009)
8000156 Heidelberg Hbf        01644  01646                % 02292 80____    01 (010)
8000377 Weinheim(Bergstr)     01659  01700                % 02292 80____    01 (011)
8000031 Bensheim              01708  01709                % 02292 80____    01 (012)
8000068 Darmstadt Hbf         01722  01724                % 02292 80____    01 (013)
8000105 Frankfurt(Main)Hbf    01740                       % 02292 80____    01 (014))";

cstr attributes_file_content = R"(
BT   0    450    11  Bordbistro#
FR   0    260    03  Fahrradmitnahme reservierungspflichtig#
G    0    260    05  Fahrradmitnahme begrenzt möglich#)";

cstr stations_file_content = R"(
8000096 VVS Stuttgart Hbf$S$Stoccarda
8000156 VRN Heidelberg Hbf$HD
8000377 VRN Weinheim(Bergstr)
8000031 VRN Bensheim
8000068 RMV Darmstadt Hbf$DA
8000105 RMV Frankfurt(Main)Hbf$Francfort (Main))";

cstr station_coordinates_file_content = R"(
8000096   9.181635  48.784084 Stuttgart Hbf
8000156   8.675442  49.403567 Heidelberg Hbf
8000377   8.665351  49.553302 Weinheim(Bergstr)
8000031   8.616717  49.681329 Bensheim
8000068   8.629636  49.872503 Darmstadt Hbf
8000105   8.663789  50.107145 Frankfurt(Main)Hbf)";

cstr bitfields_file_content = R"(
002687 ffffffffffffffffffffffffffffffffffffffe7cf9f1e7fffffffffffffffffffffffffffffffffffffffffffff0000
596114 ffffffffffffffffffffffffffffffffffffffe7cf9f1c7fffffffffffffffffffffffffffffffffffffffffffff0000
304723 ffffffffffffffffffffffffffffffffffffffe7cf9f127fffffffffffffffffffffffffffffffffffffffffffff0000)";

cstr platforms_rules_file_content = R"(
8000031 02292 80____ 1             000000
8000068 02292 80____ 5             000000
8000096 02292 80____ 7             000000
8000105 02292 80____ 10       1740 596114
8000105 02292 80____ 12       1740 304723
8000156 02292 80____ 3             000000
8000377 02292 80____ 2             000000)";

TEST_CASE("parse_trains") {
  try {
    flatbuffers::FlatBufferBuilder b;

    auto bitfields =
        parse_bitfields({BITFIELDS_FILE, bitfields_file_content}, b);
    auto attributes =
        parse_attributes({ATTRIBUTES_FILE, attributes_file_content}, b);
    auto stations =
        parse_stations({STATIONS_FILE, stations_file_content},
                       {COORDINATES_FILE, station_coordinates_file_content}, b);
    auto platforms = parse_platform_rules(
        {PLATFORMS_FILE, platforms_rules_file_content}, bitfields, b);

    std::vector<Offset<Train>> trains;
    parse_trains({"trains.101", trains_file_content}, stations, attributes,
                 bitfields, platforms, b, trains);

    b.Finish(CreateSchedule(b, b.CreateVector(trains),
                            b.CreateVector(values(stations)),
                            b.CreateVector(values(attributes)), {}));
  } catch (parser_error const& e) {
    printf("parser error: %s:%d\n", e.filename, e.line_number);
  }
}

}  // hrd
}  // loader
}  // motis
