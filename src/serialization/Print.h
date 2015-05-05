#ifndef TD_SERIALIZATION_PRINT_H_
#define TD_SERIALIZATION_PRINT_H_

#include <ostream>

#include "Connection.h"
#include "Nodes.h"
#include "Edges.h"

namespace td {

void printEdge(Edge const& edge, std::ostream& out, int indent);
void printNode(Node const& node, std::ostream& out, int indent, bool printRecursive);

struct indentation
{
  indentation(unsigned indent) : indent(indent) {}

  friend std::ostream& operator<<(std::ostream& out, indentation const& ind)
  {
    for (unsigned i = 0; i < ind.indent; ++i)
      out << "  ";
    return out;
  }

  unsigned indent;
};

inline std::ostream& operator<<(std::ostream& out, LightConnection const& con)
{
  return out << "dep=" << con.dTime << ", "
             << "arr=" << con.aTime << ", "
             << "trainNr=" << con._fullCon->conInfo->trainNr;
}

inline void printNode(Node const& node, std::ostream& out, int indent, bool printRecursive)
{
  out << "node_id=" << node._id << " [#edges=" << node._edges.size() << "]:\n";
  for (unsigned i = 0; i < node._edges.size(); ++i)
  {
    out << indentation(indent + 1) << "edge[" << i << "]: ";
    printEdge(node._edges._el[i], out, indent + 1);

    if (printRecursive)
    {
      out << indentation(indent + 2);
      printNode(*node._edges._el[i]._to, out, indent+2, false);
    }
  }
}

inline void printEdge(Edge const& edge, std::ostream& out, int indent) {
  out << "to=" << edge._to->_id << ", ";
  switch(edge._m._type)
  {
    case Edge::Type::ROUTE_EDGE:
    {
      out << "type=route_edge, "
          << "[#connections=" << edge._m._routeEdge._conns.size() << "]:\n";
      for (unsigned i = 0; i < edge._m._routeEdge._conns.size(); ++i)
      {
        out << indentation(indent + 1) << "connection[" << i << "]: "
            << edge._m._routeEdge._conns._el[i] << "\n";
      }
      break;
    }

    case Edge::Type::FOOT_EDGE:
      out << "type=foot_edge, "
          << "duration=" << edge._m._footEdge._timeCost << ", "
          << "transfer=" << std::boolalpha << edge._m._footEdge._transfer << ", "
          << "slot=" << static_cast<int>(edge._m._footEdge._slot)
          << "\n";
      break;

    case Edge::Type::INVALID_EDGE:
      out << "NO_EDGE\n";
      break;

    default:
      out << "UNKOWN [type=" << static_cast<int>(edge._m._type) << "]\n";
  }
}

}  // namespace td

#endif  // TD_SERIALIZATION_PRINT_H_
