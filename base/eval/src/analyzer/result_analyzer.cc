#include <fstream>
#include <iostream>
#include <vector>

#include "motis/module/message.h"
#include "motis/eval/analyzer/quantile.h"
#include "motis/protocol/RoutingResponse_generated.h"

using namespace motis;
using namespace motis::module;
using namespace motis::routing;
using namespace motis::eval;

struct response {
  response(RoutingResponse const* r)
      : labelsUntilFirst(0),
        labelsAfterLast(0),
        labelsCreated(0),
        time(r->pareto_dijkstra_timing()),
        startLabels(0),
        conCount(r->connections()->size()) {}

  unsigned labelsUntilFirst;
  unsigned labelsAfterLast;
  unsigned labelsCreated;
  unsigned time;
  unsigned startLabels;
  unsigned conCount;
  bool maxLabelQuit;
};

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "usage: " << argv[0] << " {results.txt file}\n";
    return 0;
  }

  std::vector<response> responses;

  std::ifstream in(argv[1]);
  std::string line;

  unsigned maxLabelQuit = 0;
  unsigned sum = 0;
  unsigned count = 0;
  unsigned noConCount = 0;
  unsigned labelsPoppedUntilFirstResult = 0;
  unsigned labelsPoppedAfterLastResult = 0;
  uint64_t noLabelsCreated = 0;
  unsigned startLabels = 0;

  while (in.peek() != EOF && !in.eof()) {
    std::getline(in, line);

    auto res_msg = make_msg(line);
    response res(motis_content(RoutingResponse, res_msg));

    if (res.maxLabelQuit) {
      ++maxLabelQuit;
    }

    if (res.conCount == 0) {
      ++noConCount;
      continue;
    }

    responses.push_back(res);
    labelsPoppedUntilFirstResult += res.labelsUntilFirst;
    labelsPoppedAfterLastResult += res.labelsAfterLast;
    noLabelsCreated += res.labelsCreated;
    sum += res.time;
    startLabels += res.startLabels;
    ++count;
  }

  if (responses.empty()) {
    std::cout << "no responses found\n";
    std::cout << "responses with no connection = " << noConCount << "\n";
    return 0;
  }

  std::cout << "   total count: " << count << "\n"
            << "       no conn: " << noConCount << "\n"
            << "max label quit: " << maxLabelQuit << "\n"
            << "\n\n";

  std::cout << "    average core routing time [ms]: "
            << sum / static_cast<double>(count) << "\n"
            << "90 quantile core routing time [ms]: "
            << quantile(&response::time, responses, 0.9f) << "\n"
            << "80 quantile core routing time [ms]: "
            << quantile(&response::time, responses, 0.8f) << "\n\n";

  std::cout << "    average number of start labels: "
            << startLabels / static_cast<double>(count) << "\n"
            << "90 quantile number of start labels: "
            << quantile(&response::startLabels, responses, 0.9f) << "\n"
            << "80 quantile number of start labels: "
            << quantile(&response::startLabels, responses, 0.8f) << "\n\n";

  std::cout << "    average number of labels created: "
            << noLabelsCreated / static_cast<double>(count) << "\n"
            << "90 quantile number of labels created: "
            << quantile(&response::labelsCreated, responses, 0.9f) << "\n"
            << "80 quantile number of labels created: "
            << quantile(&response::labelsCreated, responses, 0.8f) << "\n\n";

  std::cout << "avg labels popped:\n";
  std::cout << "\tuntil first result: "
            << labelsPoppedUntilFirstResult / static_cast<double>(count) << " ("
            << labelsPoppedUntilFirstResult /
                   static_cast<double>(noLabelsCreated)
            << ")\n";
  std::cout << "\t after last result: "
            << labelsPoppedAfterLastResult / static_cast<double>(count) << " ("
            << labelsPoppedAfterLastResult /
                   static_cast<double>(noLabelsCreated)
            << ")\n";
}