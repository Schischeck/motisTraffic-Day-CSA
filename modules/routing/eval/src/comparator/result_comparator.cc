#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <vector>

#include "motis/module/message.h"
#include "motis/eval/comparator/response.h"
#include "motis/protocol/RoutingRequest_generated.h"

using namespace motis;
using namespace motis::routing;
using namespace motis::module;
using namespace motis::eval::comparator;

template <typename T>
char get_relation_symbol(T const& u1, T const& u2) {
  if (u1 == u2) {
    return '=';
  } else if (u1 < u2) {
    return '<';
  } else {
    return '>';
  }
}

void print(connection const& con) {
  unsigned duration, transfers, price;
  std::tie(duration, transfers, price) = con;

  std::cout << std::setw(5) << duration << "\t"  //
            << std::setw(5) << transfers << "\t"  //
            << std::setw(5) << price;
}

void print_empty() {
  std::cout << std::setw(5) << "-"
            << "\t" << std::setw(5) << "-"
            << "\t" << std::setw(5) << "-";
}

bool print_differences(response const& r1, response const& r2,
                       RoutingRequest const*, int id) {
  if (r1.connections_ == r2.connections_) {
    return true;
  }

  std::cout << "ERROR [id = " << id << "] ";
  if (r1.connections_.size() != r2.connections_.size()) {
    std::cout << "#con1 = " << r1.connections_.size() << ", "
              << "#con2 = " << r2.connections_.size() << " ";
  } else {
    std::cout << "#con = " << r1.connections_.size() << " ";
  }

  auto end1 = end(r1.connections_);
  auto end2 = end(r2.connections_);
  {
    std::cout << "\n";
    auto it1 = begin(r1.connections_);
    auto it2 = begin(r2.connections_);
    int i = 0;
    while (true) {
      bool stop1 = false, stop2 = false;

      std::cout << "   " << std::setw(2) << i++ << ":  ";

      if (it1 != end1) {
        print(*it1);
        ++it1;
      } else {
        print_empty();
        stop1 = true;
      }

      std::cout << "\t\t";

      if (it2 != end2) {
        print(*it2);
        ++it2;
      } else {
        print_empty();
        stop2 = true;
      }

      std::cout << "\n";

      if (stop1 && stop2) {
        break;
      }
    }
    std::cout << "\n";
  }

  std::vector<bool> matches1(r1.connections_.size()),
      matches2(r2.connections_.size());
  bool con1_dominates = false, con2_dominates = false;
  unsigned con_count1 = 0;
  for (auto it1 = begin(r1.connections_); it1 != end1; ++it1, ++con_count1) {
    unsigned con_count2 = 0;
    for (auto it2 = begin(r2.connections_); it2 != end2; ++it2, ++con_count2) {
      if (*it1 == *it2) {
        matches1[con_count1] = true;
        matches2[con_count2] = true;
        continue;
      }

      std::string domination_info;
      if (dominates(*it1, *it2)) {
        domination_info = "\tFIRST DOMINATES \t";
        con1_dominates = true;
      } else if (dominates(*it2, *it1)) {
        domination_info = "\tSECOND DOMINATES\t";
        con2_dominates = true;
      } else {
        domination_info = "\tNO DOMINATION   \t";
        continue;
      }

      std::cout << "  " << std::setw(2) << con_count1 << " vs " << std::setw(2)
                << con_count2 << domination_info;

      int duration1, transfers1, price1, duration2, transfers2, price2;

      std::tie(duration1, transfers1, price1) = *it1;
      std::tie(duration2, transfers2, price2) = *it2;

      std::cout << "  ";
      if (duration1 != duration2) {
        std::cout << "{ DURATIONS[" << con_count1 << ", " << con_count2
                  << "]: " << duration1 << " "
                  << get_relation_symbol(duration1, duration2) << " "
                  << duration2 << " } ";
      }

      if (transfers1 != transfers2) {
        std::cout << "{ TRANSFERS[" << con_count1 << ", " << con_count2
                  << "]: " << transfers1 << " "
                  << get_relation_symbol(transfers1, transfers2) << " "
                  << transfers2 << " } ";
      }

      if (price1 != price2) {
        std::cout << "{ PRICES[" << con_count1 << ", " << con_count2
                  << "]: " << price1 << " "
                  << get_relation_symbol(price1, price2) << " " << price2
                  << " }";
      }

      std::cout << "\n";
    }
  }

  std::cout << "\n  -> connections in FIRST with no match in SECOND: ";
  bool match1 = false;
  for (unsigned i = 0; i < matches1.size(); i++) {
    if (!matches1[i]) {
      std::cout << i << " ";
      match1 = true;
    }
  }
  if (!match1) {
    std::cout << "-";
  }

  std::cout << "\n  -> connections in SECOND with no match in FIRST: ";
  bool match2 = false;
  for (unsigned i = 0; i < matches2.size(); i++) {
    if (!matches2[i]) {
      std::cout << i << " ";
      match2 = true;
    }
  }
  if (!match2) {
    std::cout << "-";
  }

  if (con1_dominates && !con2_dominates) {
    std::cout << "\n  -> total domination by FIRST\n\n\n";
  } else if (con2_dominates && !con1_dominates) {
    std::cout << "\n  -> total domination by SECOND\n\n\n";
  } else {
    std::cout << "\n  -> no total domination\n\n\n";
  }

  return false;
}

void write_file(std::string const& content, std::string const& filename) {
  std::ofstream out(filename);
  out << content << "\n";
}

struct statistics {
  statistics() : matches_(0), mismatches_(0) {}
  int total() const { return matches_ + mismatches_; }
  int matches_, mismatches_;
};

bool analyze_result(int i, std::tuple<msg_ptr, msg_ptr, msg_ptr> const& res,
                    std::ofstream& failed_queries, statistics& stats) {
  if (!std::get<0>(res) || !std::get<1>(res) || !std::get<2>(res)) {
    return false;
  }

  auto const& q = std::get<0>(res);
  auto const& r1 = std::get<1>(res);
  auto const& r2 = std::get<2>(res);

  if (print_differences(response(motis_content(RoutingResponse, r1)),
                        response(motis_content(RoutingResponse, r2)),
                        motis_content(RoutingRequest, q), i)) {
    ++stats.matches_;
  } else {
    ++stats.mismatches_;
    failed_queries << q->to_json() << "\n";
    write_file(r1->to_json(),
               "fail_responses/" + std::to_string(i) + "_1.json");
    write_file(r2->to_json(),
               "fail_responses/" + std::to_string(i) + "_2.json");
    write_file(q->to_json(), "fail_queries/" + std::to_string(i) + ".json");
  }

  return true;
}

int main(int argc, char* argv[]) {
  if (argc != 4) {
    std::cout << "Usage: " << argv[0]
              << " {results.txt I} {results.txt II} {queries.txt}\n";
    return 0;
  }

  statistics stats;
  std::ifstream in1(argv[1]), in2(argv[2]), inq(argv[3]);
  std::ofstream failed_queries("failed_queries.txt");
  std::string line1, line2, lineq;
  std::map<int, std::tuple<msg_ptr, msg_ptr, msg_ptr>> pending_msgs;
  while (in1.peek() != EOF && !in1.eof() && in2.peek() != EOF && !in2.eof() &&
         inq.peek() != EOF && !inq.eof()) {
    std::getline(in1, line1);
    std::getline(in2, line2);
    std::getline(inq, lineq);

    auto const r1 = make_msg(line1);
    auto const r2 = make_msg(line2);
    auto const q = make_msg(lineq);

    std::get<0>(pending_msgs[q->id()]) = q;
    std::get<1>(pending_msgs[r1->id()]) = r1;
    std::get<2>(pending_msgs[r2->id()]) = r2;

    for (auto const i : {q->id(), r1->id(), r2->id()}) {
      auto const it = pending_msgs.find(i);
      if (it == end(pending_msgs)) {
        continue;
      }

      if (analyze_result(i, pending_msgs.at(i), failed_queries, stats)) {
        pending_msgs.erase(i);
      }
    }
  }

  if (!pending_msgs.empty()) {
    std::cout << "warning: " << pending_msgs.size()
              << " unmatched queries/responses:\n";
    for (auto const& m : pending_msgs) {
      auto const& id = m.first;
      auto const& v = m.second;

      std::cout << "  id=" << id << ": "  //
                << "query_set=" << std::boolalpha
                << static_cast<bool>(std::get<0>(v)) << " "
                << "res1_set=" << std::boolalpha
                << static_cast<bool>(std::get<1>(v)) << " "
                << "res2_set=" << std::boolalpha
                << static_cast<bool>(std::get<2>(v)) << "\n";
    }
  }

  std::cout << "\nStatistics:\n"
            << "  #matches = " << stats.matches_ << "/" << stats.total() << "\n"
            << "  #errors  = " << stats.mismatches_ << "/" << stats.total()
            << "\n";
}
