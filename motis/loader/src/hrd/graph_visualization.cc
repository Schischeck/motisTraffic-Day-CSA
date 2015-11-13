#include "motis/loader/hrd/graph_visualization.h"

#include <sstream>
#include <fstream>
#include <string>
#include <iostream>
#include <functional>

#include "motis/loader/util.h"

namespace motis {
namespace loader {
namespace hrd {

graph_visualization::graph_visualization(node const* root) : root_(root) {}

void collect_components(node const* root,
                        std::function<int(node const*)> const& get_id,
                        std::set<std::string>& graph) {
  auto const parent_id = get_id(root);

  if (service_node const* sn = dynamic_cast<service_node const*>(root)) {
    std::stringstream label;
    label << parent_id << " [label=\"" << sn->service_->origin_.filename << " "
          << sn->service_->origin_.line_number_from << " "
          << sn->service_->origin_.line_number_to << ","
          << sn->service_->sections_[0].train_num << " "
          << sn->service_->sections_[0].admin.to_str().c_str() << "\"];\n";
    graph.insert(label.str());
  }

  for (auto const* child : root->children_) {
    if (child) {
      std::stringstream edge;
      edge << parent_id << " -- " << get_id(child) << ";\n";
      graph.insert(edge.str());
      collect_components(child, get_id, graph);
    }
  }
}

void collect_components(node const* root, std::set<std::string>& graph) {
  int next_id = 0;
  std::map<node const*, int> ids;
  auto const get_id = [&next_id, &ids](node const* n) {
    return get_or_create(ids, n, [&next_id]() { return ++next_id; });
  };
  collect_components(root, get_id, graph);
}

void graph_visualization::print() {

  std::set<std::string> graph;
  collect_components(root_, graph);

  printf("graph {\n");
  for (auto const& component : graph) {
    printf("%s", component.c_str());
  }
  printf("}\n");
}

void graph_visualization::serialize(const char* root, int id) {
  std::stringstream filename;
  filename << root << "/graph." << id << ".dot";
  std::ofstream out(filename.str());

  std::set<std::string> graph;
  collect_components(root_, graph);

  out << "graph {\n";
  for (auto const& component : graph) {
    out << component;
  }
  out << "}\n";
  out.close();
}

}  // hrd
}  // loader
}  // motis
