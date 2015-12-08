#pragma once

namespace motis {

void print_edge(edge const& edge, std::ostream& out, int indent);
void print_node(node const& node, std::ostream& out, int indent,
                bool print_recursive);

struct indentation {
  indentation(unsigned indent) : indent(indent) {}

  friend std::ostream& operator<<(std::ostream& out, indentation const& ind) {
    for (unsigned i = 0; i < ind.indent; ++i) out << "  ";
    return out;
  }

  unsigned indent;
};

inline std::ostream& operator<<(std::ostream& out,
                                light_connection const& con) {
  return out << "dep=" << con.d_time << ", "
             << "arr=" << con.a_time << ", "
             << "train_nr=" << con._full_con->con_info->train_nr;
}

inline void print_node(node const& node, std::ostream& out, int indent,
                       bool print_recursive) {
  out << "node_id=" << node._id;
  if (node.is_route_node()) {
    out << " [route=" << node._route << "]";
  }
  out << " [#edges=" << node._edges.size() << "]:\n";
  for (unsigned i = 0; i < node._edges.size(); ++i) {
    out << indentation(indent + 1) << "edge[" << i << "]: ";
    print_edge(node._edges._el[i], out, indent + 1);

    if (print_recursive) {
      out << indentation(indent + 2);
      print_node(*node._edges._el[i]._to, out, indent + 2, false);
    }
  }
}

inline void print_edge(edge const& edge, std::ostream& out, int indent) {
  out << "to=" << edge._to->_id << ", ";
  switch (edge._m._type) {
    case edge::ROUTE_EDGE: {
      out << "type=route_edge, "
          << "[#connections=" << edge._m._route_edge._conns.size() << "]:\n";
      for (unsigned i = 0; i < edge._m._route_edge._conns.size(); ++i) {
        out << indentation(indent + 1) << "connection[" << i
            << "]: " << edge._m._route_edge._conns._el[i] << "\n";
      }
      break;
    }

    case edge::FOOT_EDGE:
      out << "type=foot_edge, "
          << "duration=" << edge._m._foot_edge._time_cost << ", "
          << "transfer=" << std::boolalpha << edge._m._foot_edge._transfer
          << ", "
          << "slot=" << static_cast<int>(edge._m._foot_edge._slot) << "\n";
      break;

    case edge::INVALID_EDGE: out << "NO_EDGE\n"; break;

    default: out << "UNKOWN [type=" << static_cast<int>(edge._m._type) << "]\n";
  }
}

}  // namespace motis
