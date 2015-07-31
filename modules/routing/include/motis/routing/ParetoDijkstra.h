#ifndef TDPARETO_DIJKSTRA_H
#define TDPARETO_DIJKSTRA_H TDPARETO_DIJKSTRA_H

#include <queue>
#include <list>
#include <ostream>
#include <unordered_map>

#include "motis/routing/Label.h"
#include "motis/routing/LowerBounds.h"

namespace td {

class Heuristic;
class Node;
class StationNode;

class ParetoDijkstra
{
public:
  class CompareLabels
  {
    public:
      bool operator() (const Label* l1, const Label* l2) const
      { return l2->operator<(*l1); }
  };

  /** stores search statistics for performance analysis */
  struct Statistics
  {
    Statistics()
      : labelsPopped(0), labelsCreated(0),
        labelsDominatedByResults(0), labelsFiltered(0),
        labelsDominatedByFormerLabels(0), labelsDominatedByLaterLabels(0),
        labelsPoppedUntilFirstResult(-1),
        labelsPoppedAfterLastResult(0),
        priorityQueueMaxSize(0),
        startLabelCount(0),
        labelsEqualsPopped(0),
        maxLabelQuit(false)
    {}

    int labelsPopped, labelsCreated, labelsDominatedByResults;
    int labelsFiltered, labelsDominatedByFormerLabels,
        labelsDominatedByLaterLabels;
    int labelsPoppedUntilFirstResult, labelsPoppedAfterLastResult;
    int priorityQueueMaxSize;
    int startLabelCount;
    int labelsEqualsPopped;
    bool maxLabelQuit;
    int travelTimeLB;
    int transfersLB;
    int priceLB;
    int totalCalculationTime;

    friend std::ostream& operator<<(std::ostream& o, Statistics const& s) {
      return
      o << "Stats:\n"
        << "\ttravelTimeLB: " << s.travelTimeLB << "ms\n"
        << "\ttransfersLB: " << s.transfersLB << "ms\n"
        << "\tpriceLB: " << s.priceLB << "ms\n"
        << "\tstartLabelCount:" << s.startLabelCount << "\n"
        << "\tcreated: " << s.labelsCreated << "\n"
        << "\tpopped: " << s.labelsPopped << "\n"
        << "\tequalsPopped:" << s.labelsEqualsPopped << "\n"
        << "\tdominatedByResults:" << s.labelsDominatedByResults << "\n"
        << "\tdominatedByFormerLabels:" << s.labelsDominatedByFormerLabels << "\n"
        << "\tpoppedUntilFirstResult:" << s.labelsPoppedUntilFirstResult << "\n"
        << "\tpoppedAfterLastResult:" << s.labelsPoppedAfterLastResult << "\n"
        << "\tpriorityQueueMaxSize:" << s.priorityQueueMaxSize << "\n"
        << "\tfiltered:" << s.labelsFiltered << "\n"
        << "\tmaxLabelQuit:" << std::boolalpha << s.maxLabelQuit << "\n"
        << "\ttotalCalculationTime:" << s.totalCalculationTime << "\n";
    }
  };

  ParetoDijkstra(
      int nodeCount,
      StationNode const* goal,
      std::vector<Label*> const& startLabels,
      std::unordered_map<Node const*, std::vector<Edge>> const& additionalEdges,
      LowerBounds& lowerBounds,
      MemoryManager<Label>& labelStore);

  std::vector<Label*>& search();
  Statistics getStatistics() const;

private:
  void createNewLabel(Label* label, Edge const& edge);
  bool addResult(Label* li);
  bool dominatedByResults(Label* label);
  bool addLabelToNode(Label* label, Node const* dest);
  void filterResults();

  StationNode const* _goal;
  std::vector<std::vector<Label*>> _nodeLabels;
  std::priority_queue<Label*, std::vector<Label*>, CompareLabels> _queue;
  std::vector<Label*> _equals;
  std::unordered_map<Node const*, std::vector<Edge>> _additionalEdges;
  std::vector<Label*> _results;
  LowerBounds& _lowerBounds;
  MemoryManager<Label>& _labelStore;
  Statistics _stats;
};

}  // namespace td

#endif //TDPARETO_DIJKSKTRA_H

