#include <iostream>
#include <fstream>

#include "pugixml.hpp"

#include "conf/options_parser.h"

#include "Label.h"
#include "Query.h"
#include "Graph.h"
#include "serialization/Schedule.h"
#include "execution/dataset_settings.h"
#include "execution/channel.h"
#include "execution/parallel_settings.h"
#include "execution/query_file_settings.h"

using namespace td;
using namespace td::execution;

struct response {
  bool error;
  std::string xml;
  std::string query;
};

typedef channel<std::string> input_channel;
typedef channel<response> output_channel;

void worker(Schedule& schedule,
            StationGuesser& guesser,
            input_channel& in,
            output_channel& out) {
  MemoryManager<Label> labelStore(MAX_LABELS_WITH_MARGIN);
  Graph graph(schedule, labelStore);

  channel<std::string>::queue_element queueEntry;
  while (in[in.any] >> queueEntry) {
    auto const& q = queueEntry.value;

    pugi::xml_document query_doc;
    pugi::xml_parse_result result = query_doc.load(q.c_str());
    if (!result) {
      std::ostringstream error;
      error << "Parse error (" << result.description() << ")\n"
            << "Offset: " << result.offset << "\n"
            << "At: [..." << q.substr(result.offset, 100) << "]\n";
      out[queueEntry.topic] << response { true, error.str(), q };
      continue;
    }

    Query query(graph, guesser);
    query.initFromXML(query_doc, true);

    pugi::xml_document responseDoc;
    bool error = query.execute(responseDoc);

    std::stringstream responseString;
    pugi::xml_node decl = responseDoc.prepend_child(pugi::node_declaration);
    decl.append_attribute("version") = "1.0";
    decl.append_attribute("encoding") = "UTF-8";
    decl.append_attribute("standalone") = "no";
    responseDoc.save(responseString, "", pugi::format_raw);

    out[queueEntry.topic] << response { error, responseString.str(), q };
  }
}

void batch_input(input_channel& ch, std::ifstream& file) {
  int queryId = 0;
  while (file.peek() != EOF) {
    try {
      std::string query;
      std::getline(file, query);
      ch[++queryId] << query;
    } catch (std::ios_base::failure const& e) {
      std::cerr << "I/O failure: \"" << e.what() << "\", exiting\n";
      break;
    }
  }
  ch.stop();
}

void batch_output(output_channel& ch) {
  auto all_responses = std::make_shared<std::ofstream>("responses.txt");
  auto successful_responses = std::make_shared<std::ofstream>("successful_responses.txt");
  auto successful_queries = std::make_shared<std::ofstream>("successful_queries.txt");
  ch[ch.any].listen([=](output_channel::queue_element const& response) {
    auto const& result = response.value;

    *all_responses << response.topic << "." << result.xml << "\n";

    if (!result.error) {
      *successful_responses << response.topic << "." << result.xml << "\n";
      *successful_queries << response.topic << "." << result.query << "\n";
    }

    return false;
  });
}


int main(int argc, char* argv[]) {
  dataset_settings dataset_opt("data/test");
  query_file_settings query_file_opt("queries.txt");
  parallel_settings parallel_opt;

  conf::options_parser parser({ &dataset_opt, &query_file_opt, &parallel_opt });
  parser.read_command_line_args(argc, argv);

  if (parser.help()) {
    std::cout << "\n\tTime Dependent Routing\n\n";
    parser.print_help(std::cout);
    return 0;
  } else if (parser.version()) {
    std::cout << "Time Dependent Routing\n";
    return 0;
  }

  parser.read_configuration_file();

  std::cout << "\n\tTime Dependent Routing\n\n";
  parser.print_unrecognized(std::cout);
  parser.print_used(std::cout);

  td::Schedule sched(dataset_opt.dataset);
  td::StationGuesser guesser(sched.stations);

  input_channel in(10);
  output_channel out;

  std::vector<std::thread> threads(parallel_opt.num_threads);
  for (int i = 0; i < parallel_opt.num_threads; ++i)
    threads[i] = std::thread([&]() { worker(sched, guesser, in, out); });

  std::ifstream query_file(query_file_opt.query_file.c_str());
  query_file.exceptions(std::ios_base::failbit);

  batch_output(out);
  batch_input(in, query_file);

  std::for_each(begin(threads), end(threads), [](std::thread& t) { t.join(); });
}
