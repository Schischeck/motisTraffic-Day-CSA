#pragma once

#include "motis/routes/prepare/rel/polyline_aggregator.h"
#include "motis/routes/prepare/rel/relation_parser.h"
#include "geo/latlng.h"

using namespace geo;

namespace motis {
namespace routes {

void test_aggregator_cycle() {
  node n1 = node(0);
  node n2 = node(1);
  node n3 = node(2);
  node n4 = node(3);
  node n5 = node(4);
  n1.pos_ = latlng(0, 1);
  n2.pos_ = latlng(0, 2);
  n3.pos_ = latlng(0, 3);
  n4.pos_ = latlng(0, 4);
  n5.pos_ = latlng(0, 5);

  way w1 = way(0);
  way w2 = way(1);
  way w3 = way(2);
  way w4 = way(3);
  // n1 -> n2  n2 -> n3 n3 -> n4 n4 -> n2
  w1.nodes_.push_back(n1);
  w1.nodes_.push_back(n2);
  w2.nodes_.push_back(n2);
  w2.nodes_.push_back(n3);
  w3.nodes_.push_back(n3);
  w3.nodes_.push_back(n4);
  w4.nodes_.push_back(n4);
  w4.nodes_.push_back(n3);

  std::vector<way*> ways = {&w1, &w2, &w3, &w4};

  relation r1 = relation(source_spec(0, source_spec::category::RAILWAY), ways);
  aggregate_polylines({r1});
}

}  // namespace routes
}  // namespace motis
