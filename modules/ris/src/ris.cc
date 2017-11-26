#include "motis/ris/ris.h"

#include <optional>

#include "boost/filesystem.hpp"

#include "conf/date_time.h"
#include "conf/simple_config_param.h"

#include "tar/file_reader.h"
#include "tar/tar_reader.h"
#include "tar/zstd_reader.h"

#include "lmdb/lmdb.hpp"

#include "motis/module/context/motis_spawn.h"
#include "motis/ris/risml/risml_parser.h"
#include "motis/ris/zip_reader.h"

namespace fs = boost::filesystem;
namespace db = lmdb;
using namespace motis::module;
using motis::ris::risml::xml_to_ris_message;
using tar::file_reader;
using tar::tar_reader;
using tar::zstd_reader;

namespace motis {
namespace ris {

// stores the list of files that were already parsed
// key: path
// value: empty
constexpr auto const FILE_DB = "FILE_DB";

// messages, sorted by timestamp
// key: message id
// value: message buffer
constexpr auto const MSG_DB = "MSG_DB";

// index for every day referenced by any message
// key: day.begin (unix timestamp)
// value: smallest message id from MSG_DB that has
//        earliest <= day.begin && latest >= day.end
constexpr auto const MIN_DAY = "MIN_DAY";

// index for every day referenced by any message
// key: day.begin (unix timestamp)
// value: largest message id from MSG_DB that has
//        earliest <= day.begin && latest >= day.end
constexpr auto const MAX_DAY = "MAX_DAY";

struct ris::impl {
  void init() {
    if (fs::is_directory(db_path_)) {
      env_.set_maxdbs(2);
      env_.open(db_path_.c_str(), db::env_open_flags::NOMEMINIT);

      {
        db::txn t{env_};
        t.dbi_open(FILE_DB, db::dbi_flags::CREATE);
        t.dbi_open(MSG_DB, db::dbi_flags::CREATE | db::dbi_flags::INTEGERKEY);
        t.dbi_open(MIN_DAY, db::dbi_flags::CREATE | db::dbi_flags::INTEGERKEY);
        t.dbi_open(MAX_DAY, db::dbi_flags::CREATE | db::dbi_flags::INTEGERKEY);
        t.commit();
      }
    }

    // TODO(felix)
    // parse_folder()
    // forward(new_time=init_time_)
  }

  msg_ptr upload(msg_ptr const&) {
    // TODO(felix)
    // write to folder()
    // parse_folder()
    // forward()
    return {};
  }

  msg_ptr forward(msg_ptr const&) {
    // TODO(felix)
    /*
      cursor c;
      std::vector<parser::buffer> blobs;
      for (auto const& m : c) {
        if (m->timstamp() > batch_to) {
          publish(blobs);
          blobs.clear();
        }
        blobs.emplace_back(m);
      }
    */
    return {};
  }

  std::string db_path_;
  std::string input_folder_;
  conf::holder<std::time_t> init_time_{0};

private:
  enum class file_type { NONE, ZST, ZIP, XML };

  static file_type get_file_type(fs::path const& p) {
    if (p.extension() == ".zst") {
      return file_type::ZST;
    } else if (p.extension() == ".zip") {
      return file_type::ZIP;
    } else if (p.extension() == ".xml") {
      return file_type::XML;
    } else {
      return file_type::NONE;
    }
  }

  void parse_folder(fs::path const& p) {
    auto const root = p.root_path();
    std::vector<std::shared_ptr<ctx::future<ctx_data, void>>> futures;
    for (auto const& it : fs::directory_iterator(p)) {
      auto const& rel_path = it.path();
      if (fs::is_regular_file(p)) {
        auto const p = fs::canonical(rel_path, root);
        if (auto const t = get_file_type(p);
            t != file_type::NONE && !is_known_file(p)) {
          futures.emplace_back(
              spawn_job_void([p, t, this]() { write_to_db(p, t); }));
        }
      } else if (fs::is_directory(p)) {
        parse_folder(p);
      }
    }

    for (auto& f : futures) {
      f->val();
    }
  }

  bool is_known_file(fs::path const&) {
    // TODO(felix)
    return false;
  }

  void write_to_db(ris_message&&) {
    // TODO(felix)
  }

  void write_to_db(fs::path const& p, file_type const type) {
    auto parse = [&](auto&& reader) {
      std::optional<std::string_view> xml;
      while ((xml = reader.read())) {
        xml_to_ris_message(*xml,
                           [&](ris_message&& m) { write_to_db(std::move(m)); });
      }
    };

    auto cp = p.c_str();
    switch (type) {
      case file_type::ZST:
        parse(tar_reader<zstd_reader>{zstd_reader{cp}});
        break;
      case file_type::ZIP: parse(zip_reader{cp}); break;
      case file_type::XML: parse(file_reader{cp}); break;
      default: assert(false);
    }
  }

  lmdb::env env_;
};

ris::ris() : module("RIS", "ris"), impl_(std::make_unique<impl>()) {
  string_param(impl_->db_path_, "ris_db", "db", "ris database path");
  string_param(impl_->input_folder_, "ris", "input_folder", "ris input folder");
  template_param(impl_->init_time_, {0l}, "init_time",
                 "'forward' the simulation clock (expects Unix timestamp)");
}

ris::~ris() = default;

void ris::init(motis::module::registry& r) {
  r.subscribe_void("/init", [this] { impl_->init(); });
  r.register_op("/ris/upload", [this](auto&& m) { return impl_->upload(m); });
  r.register_op("/ris/forward", [this](auto&& m) { return impl_->forward(m); });
}

}  // namespace ris
}  // namespace motis
