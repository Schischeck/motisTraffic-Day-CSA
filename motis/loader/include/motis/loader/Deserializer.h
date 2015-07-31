#ifndef TD_SERIALIZATION_DESERIALIZER_H_
#define TD_SERIALIZATION_DESERIALIZER_H_

#include <memory>
#include <vector>
#include <string>

#include "motis/core/common/Array.h"
#include "motis/core/common/Offset.h"
#include "motis/core/schedule/Connection.h"
#include "motis/core/schedule/Station.h"
#include "motis/core/schedule/Nodes.h"

namespace td {

class Deserializer {
public:
  Deserializer(std::string const& prefix);

  std::pair<int, std::unique_ptr<char[]>>
  loadGraph(std::vector<StationPtr>& stations,
            std::vector<StationNodePtr>& stationNodes);

private:
  std::string _prefix;
};

}  // namespace td

#endif  // TD_SERIALIZATION_DESERIALIZER_H_
