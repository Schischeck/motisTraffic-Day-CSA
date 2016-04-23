#pragma once

#include <map>

namespace motis {
namespace loader {

inline std::map<std::string, int> class_mapping() {
  // clang-format off
  return {
    // high speed
    { "ICE", 0},
    { "THA", 0},
    { "TGV", 0},
    { "RJ", 0},

    // long range
    { "EC", 1 },
    { "IC", 1 },
    { "EX", 1 },
    { "D", 1 },

    // night trains
    { "CNL", 2 },
    { "EN", 2 },
    { "AZ", 2 },

    // fast local trains
    { "IRE", 3 },
    { "REX", 3 },
    { "RE", 3 },
    { "IR", 3 },
    { "X", 3 },
    { "DPX", 3 },
    { "E", 3 },
    { "Sp", 3 },

    // local trains
    { "DPN", 4 },
    { "R", 4 },
    { "DPF", 4 },
    { "RB", 4 },
    { "Os", 4 },

    // metro
    { "S", 5 },
    { "SB", 5 },

    // subway
    { "U", 6 },
    { "STB", 6 },

    // street-car
    { "STR", 7 },

    // bus
    { "Bus", 8 },

    // other
    { "Flug", 9 },
    { "Schiff", 9 },
    { "ZahnR", 9 },
    { "Schw-B", 9 },
    { "Fähre", 9 },
    { "KAT", 9 },
    { "EZ", 9 },
    { "ALT", 9 },
    { "AST", 9 },
    { "RFB", 9 },
    { "RT", 9 }
  };
  // clang-format on
}

}  // namespace loader
}  // namespace motis
