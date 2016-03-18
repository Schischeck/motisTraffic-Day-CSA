#include "motis/ris/mode/base_mode.h"

#include "motis/core/common/logging.h"
#include "motis/ris/database.h"
#include "motis/ris/detail/find_new_files.h"
#include "motis/ris/ris.h"
#include "motis/ris/risml_parser.h"
#include "motis/ris/zip_reader.h"

using namespace motis::module;
using namespace motis::logging;
using namespace motis::ris::detail;

namespace motis {
namespace ris {
namespace mode {

void base_mode::init_async() {
  db_init();
  read_files_ = db_get_files();

  scoped_timer timer("RIS parsing RISML into database");
  auto new_files = find_new_files(module_->input_folder_, ".zip", read_files_);

  int c = 0;
  for (auto const& new_file : new_files) {  // TODO parallelize this loop
    if (++c % 100 == 0) {
      LOG(info) << "RIS parsing " << c << "/" << new_files.size() << std::endl;
    }
    db_put_messages(new_file, parse_xmls(read_zip_file(new_file)));
  }
}

}  // namespace mode
}  // namespace ris
}  // namespace motis
