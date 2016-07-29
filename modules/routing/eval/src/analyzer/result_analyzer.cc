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
  explicit response(RoutingResponse const* r)
      : labels_until_first_(
            r->statistics()->labels_popped_until_first_result()),
        labels_after_last_(r->statistics()->labels_popped_after_last_result()),
        labels_created_(r->statistics()->labels_created()),
        time_(r->statistics()->pareto_dijkstra()),
        start_labels_(r->statistics()->start_label_count()),
        con_count_(r->connections()->size()),
        max_label_quit_(false) {}

  unsigned labels_until_first_;
  unsigned labels_after_last_;
  unsigned labels_created_;
  unsigned time_;
  unsigned start_labels_;
  unsigned con_count_;
  bool max_label_quit_;
};

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "usage: " << argv[0] << " {results.txt file}\n";
    return 0;
  }

  std::vector<response> responses;

  std::ifstream in(argv[1]);
  std::string line;

  unsigned max_label_quit = 0;
  unsigned sum = 0;
  unsigned count = 0;
  unsigned no_con_count = 0;
  unsigned labels_popped_until_first_result = 0;
  unsigned labels_popped_after_last_result = 0;
  uint64_t no_labels_created = 0;
  unsigned start_labels = 0;

  while (in.peek() != EOF && !in.eof()) {
    std::getline(in, line);

    auto res_msg = make_msg(line);
    response res(motis_content(RoutingResponse, res_msg));

    if (res.max_label_quit_) {
      ++max_label_quit;
    }

    if (res.con_count_ == 0) {
      ++no_con_count;
      continue;
    }

    responses.push_back(res);
    labels_popped_until_first_result += res.labels_until_first_;
    labels_popped_after_last_result += res.labels_after_last_;
    no_labels_created += res.labels_created_;
    sum += res.time_;
    start_labels += res.start_labels_;
    ++count;
  }

  if (responses.empty()) {
    std::cout << "no responses found\n";
    std::cout << "responses with no connection = " << no_con_count << "\n";
    return 0;
  }

  std::cout << "   total count: " << count << "\n"
            << "       no conn: " << no_con_count << "\n"
            << "max label quit: " << max_label_quit << "\n"
            << "\n\n";

  std::cout << "    average core routing time [ms]: "
            << sum / static_cast<double>(count) << "\n"
            << "90 quantile core routing time [ms]: "
            << quantile(&response::time_, responses, 0.9f) << "\n"
            << "80 quantile core routing time [ms]: "
            << quantile(&response::time_, responses, 0.8f) << "\n\n";

  std::cout << "    average number of start labels: "
            << start_labels / static_cast<double>(count) << "\n"
            << "90 quantile number of start labels: "
            << quantile(&response::start_labels_, responses, 0.9f) << "\n"
            << "80 quantile number of start labels: "
            << quantile(&response::start_labels_, responses, 0.8f) << "\n\n";

  std::cout << "    average number of labels created: "
            << no_labels_created / static_cast<double>(count) << "\n"
            << "90 quantile number of labels created: "
            << quantile(&response::labels_created_, responses, 0.9f) << "\n"
            << "80 quantile number of labels created: "
            << quantile(&response::labels_created_, responses, 0.8f) << "\n\n";

  std::cout << "avg labels popped:\n";
  std::cout << "\tuntil first result: "
            << labels_popped_until_first_result / static_cast<double>(count)
            << " ("
            << labels_popped_until_first_result /
                   static_cast<double>(no_labels_created)
            << ")\n";
  std::cout << "\t after last result: "
            << labels_popped_after_last_result / static_cast<double>(count)
            << " ("
            << labels_popped_after_last_result /
                   static_cast<double>(no_labels_created)
            << ")\n";
}
