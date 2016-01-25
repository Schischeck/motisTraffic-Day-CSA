#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>

#include "motis/module/message.h"
#include "motis/eval/comparator/response.h"
#include "motis/protocol/RoutingRequest_generated.h"

using namespace motis;
using namespace motis::routing;
using namespace motis::module;
using namespace motis::eval::comparator;

template <typename T>
char getRelationSymbol(T const& u1, T const& u2) {
  if (u1 == u2)
    return '=';
  else if (u1 < u2)
    return '<';
  else
    return '>';
}

void print(connection const& con) {
  unsigned duration, transfers, price;
  std::tie(duration, transfers, price) = con;

  std::cout << std::setw(5) << duration << "\t"  //
            << std::setw(5) << transfers << "\t"  //
            << std::setw(5) << price;
}

void printEmpty() {
  std::cout << std::setw(5) << "-"
            << "\t" << std::setw(5) << "-"
            << "\t" << std::setw(5) << "-";
}

bool printDifferences(response const& r1, response const& r2,
                      RoutingRequest const*, int line, int id) {
  if (r1.connections == r2.connections) {
    return true;
  }

  std::cout << "ERROR [line = " << line << ", id = " << id << "] ";
  if (r1.connections.size() != r2.connections.size()) {
    std::cout << "#con1 = " << r1.connections.size() << ", "
              << "#con2 = " << r2.connections.size() << " ";
  } else {
    std::cout << "#con = " << r1.connections.size() << " ";
  }

  auto end1 = end(r1.connections);
  auto end2 = end(r2.connections);
  {
    std::cout << "\n";
    auto it1 = begin(r1.connections);
    auto it2 = begin(r2.connections);
    int i = 0;
    while (true) {
      bool stop1 = false, stop2 = false;

      std::cout << "   " << std::setw(2) << i++ << ":  ";

      if (it1 != end1) {
        print(*it1);
        ++it1;
      } else {
        printEmpty();
        stop1 = true;
      }

      std::cout << "\t\t";

      if (it2 != end2) {
        print(*it2);
        ++it2;
      } else {
        printEmpty();
        stop2 = true;
      }

      std::cout << "\n";

      if (stop1 && stop2) {
        break;
      }
    }
    std::cout << "\n";
  }

  std::vector<bool> matches1(r1.connections.size()),
      matches2(r2.connections.size());
  bool con1Dominates = false, con2Dominates = false;
  unsigned conCount1 = 0;
  for (auto it1 = begin(r1.connections); it1 != end1; ++it1, ++conCount1) {
    unsigned conCount2 = 0;
    for (auto it2 = begin(r2.connections); it2 != end2; ++it2, ++conCount2) {
      if (*it1 == *it2) {
        matches1[conCount1] = true;
        matches2[conCount2] = true;
        continue;
      }

      std::string dominationInfo;
      if (dominates(*it1, *it2)) {
        dominationInfo = "\tFIRST DOMINATES \t";
        con1Dominates = true;
      } else if (dominates(*it2, *it1)) {
        dominationInfo = "\tSECOND DOMINATES\t";
        con2Dominates = true;
      } else {
        dominationInfo = "\tNO DOMINATION   \t";
        continue;
      }

      std::cout << "  " << std::setw(2) << conCount1 << " vs " << std::setw(2)
                << conCount2 << dominationInfo;

      int duration1, transfers1, price1, duration2, transfers2, price2;

      std::tie(duration1, transfers1, price1) = *it1;
      std::tie(duration2, transfers2, price2) = *it2;

      std::cout << "  ";
      if (duration1 != duration2)
        std::cout << "{ DURATIONS[" << conCount1 << ", " << conCount2
                  << "]: " << duration1 << " "
                  << getRelationSymbol(duration1, duration2) << " " << duration2
                  << " } ";

      if (transfers1 != transfers2)
        std::cout << "{ TRANSFERS[" << conCount1 << ", " << conCount2
                  << "]: " << transfers1 << " "
                  << getRelationSymbol(transfers1, transfers2) << " "
                  << transfers2 << " } ";

      if (price1 != price2)
        std::cout << "{ PRICES[" << conCount1 << ", " << conCount2
                  << "]: " << price1 << " " << getRelationSymbol(price1, price2)
                  << " " << price2 << " }";

      std::cout << "\n";
    }
  }

  std::cout << "\n  -> connections in FIRST with no match in SECOND: ";
  bool match1 = false;
  for (unsigned i = 0; i < matches1.size(); i++)
    if (!matches1[i]) {
      std::cout << i << " ";
      match1 = true;
    }
  if (!match1) std::cout << "-";

  std::cout << "\n  -> connections in SECOND with no match in FIRST: ";
  bool match2 = false;
  for (unsigned i = 0; i < matches2.size(); i++)
    if (!matches2[i]) {
      std::cout << i << " ";
      match2 = true;
    }
  if (!match2) std::cout << "-";

  if (con1Dominates && !con2Dominates)
    std::cout << "\n  -> total domination by FIRST\n\n\n";
  else if (con2Dominates && !con1Dominates)
    std::cout << "\n  -> total domination by SECOND\n\n\n";
  else
    std::cout << "\n  -> no total domination\n\n\n";

  return false;
}

void writeFile(std::string const& content, std::string const& filename) {
  std::ofstream out(filename);
  out << content << "\n";
}

int main(int argc, char* argv[]) {
  if (argc != 4) {
    std::cout << "Usage: " << argv[0]
              << " {results.txt I} {results.txt II} {queries.txt}\n";
    return 0;
  }

  unsigned matches = 0, mismatches = 0;
  unsigned lineCount = 0;
  std::ifstream in1(argv[1]), in2(argv[2]), inq(argv[3]);
  std::ofstream failedQueries("failed_queries.txt");
  std::string line1, line2, lineq;

  while (in1.peek() != EOF && !in1.eof() && in2.peek() != EOF && !in2.eof() &&
         inq.peek() != EOF && !inq.eof()) {
    std::getline(in1, line1);
    std::getline(in2, line2);
    std::getline(inq, lineq);

    auto res1 = make_msg(line1);
    auto res2 = make_msg(line2);
    auto q = make_msg(lineq);

    if (res1->content_type() != MsgContent_RoutingResponse ||
        res2->content_type() != MsgContent_RoutingResponse ||
        q->content_type() != MsgContent_RoutingRequest) {
      printf("invalid content_type(s)? skipping!\n");
      continue;
    }

    if (printDifferences(response(res1->content<RoutingResponse const*>()),
                         response(res2->content<RoutingResponse const*>()),
                         q->content<RoutingRequest const*>(), lineCount,
                         q->id())) {
      ++matches;
    } else {
      ++mismatches;
      failedQueries << lineq << "\n";
      writeFile(line1,
                "fail_responses/" + std::to_string(lineCount) + "_1.xml");
      writeFile(line2,
                "fail_responses/" + std::to_string(lineCount) + "_2.xml");
      writeFile(lineq, "fail_queries/" + std::to_string(lineCount) + ".xml");
    }

    ++lineCount;
  }

  std::cout << "\nStatistics:\n"
            << "  #matches = " << matches << "\n"
            << "  #errors  = " << mismatches << "\n";
}