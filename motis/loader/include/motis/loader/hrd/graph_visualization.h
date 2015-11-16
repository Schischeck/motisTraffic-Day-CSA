#pragma once

#include <map>

#include "motis/loader/hrd/model/rules_graph.h"

namespace motis {
namespace loader {
namespace hrd {

struct graph_visualization {

  graph_visualization(node const*);

  void print();
  void serialize(const char*, int);

private:
  node const* root_;
};

}  // hrd
}  // loader
}  // motis
