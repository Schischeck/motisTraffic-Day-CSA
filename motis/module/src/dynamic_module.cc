#include "motis/module/dynamic_module.h"

#if defined _WIN32 || defined _WIN64
#include <Windows.h>
#else  // defined _WIN32 || defined _WIN64
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#endif  // defined _WIN32 || defined _WIN64

#include "boost/filesystem.hpp"
#include "boost/algorithm/string.hpp"

namespace fs = boost::filesystem;

namespace motis {
namespace module {

std::vector<dynamic_module> modules_from_folder(std::string const& path,
                                                motis::schedule* schedule) {
  std::vector<dynamic_module> modules;

  // Check that the specified path is a directory.
  if (!boost::filesystem::is_directory(path)) {
    std::cout << path << " isn't a directory\n";
    return modules;
  }

  // Tterate specified directory.
  auto i = fs::recursive_directory_iterator(path);
  for (; i != fs::recursive_directory_iterator(); ++i) {
    // Don't load hidden files.
    if (boost::starts_with(i->path().filename().string(), ".")) {
      continue;
    }

    // Only load ".module" files.
    if (i->path().extension() != ".module") {
      continue;
    }

    try {
      modules.emplace_back(i->path().generic_string(), schedule);
    } catch (...) {
      std::cerr << "unable to load " << i->path().generic_string() << "\n";
    }
  }

  return modules;
}

dynamic_module::dynamic_module(dynamic_module&& other)
    : module_(other.module_), lib_(other.lib_) {
  other.lib_ = nullptr;
}

dynamic_module& dynamic_module::operator=(dynamic_module&& other) {
  lib_ = other.lib_;
  module_ = std::move(other.module_);
  other.lib_ = nullptr;
  return *this;
}

#if defined _WIN32 || defined _WIN64
dynamic_module::dynamic_module(const std::string& p, motis::schedule* schedule)
    : lib_(nullptr) {
  // Define module map and get_modules to simplify code.
  typedef void*(__cdecl * load_module)(void*);

  // Discover package name.
  std::string name = boost::filesystem::path(p).filename().generic_string();
  std::size_t dot_pos = name.find(".");
  if (dot_pos != std::string::npos) {
    name = name.substr(0, dot_pos);
  }

  // Load library.
  lib_ = LoadLibrary(p.c_str());
  if (nullptr == lib_) {
    throw std::runtime_error("unable to load module: not a library");
  }

  // Get address to load function.
  load_module lib_fun =
      (load_module)GetProcAddress((HINSTANCE)lib_, "load_module");
  if (nullptr == lib_fun) {
    FreeLibrary((HINSTANCE)lib_);
    throw std::runtime_error("unable to load module: load_module() not found");
  }

  // Call load function.
  auto ptr = static_cast<motis::module::module*>((*lib_fun)(schedule));
  module_ = std::shared_ptr<motis::module::module>(ptr);
}

dynamic_module::~dynamic_module() {
  // Free the module itself.
  // This calls the destructor and must
  // therefore be done *before* freeing the shared library.
  module_ = nullptr;

  // Close shared library.
  if (lib_ != nullptr) {
    FreeLibrary((HINSTANCE)lib_);
  }
}
#else  // defined _WIN32 || defined _WIN64
dynamic_module::dynamic_module(const std::string& p, motis::schedule* schedule)
    : lib_(nullptr) {
  // Discover package name.
  using boost::filesystem::path;
  std::string name = path(p).filename().generic_string();
  std::size_t dot_pos = name.find(".");
  if (dot_pos != std::string::npos) {
    name = name.substr(0, dot_pos);
  }

  // Load library.
  lib_ = dlopen(p.c_str(), RTLD_LAZY);
  if (nullptr == lib_) {
    throw std::runtime_error("unable to load module: not a library");
  }

  // Get pointer to the load function.
  void* (*lib_fun)(void*);
  *(void**)(&lib_fun) = dlsym(lib_, "load_module");
  if (nullptr == lib_fun) {
    throw std::runtime_error("unable to load module: load_module() not found");
  }

  // Read modules.
  auto ptr = static_cast<motis::module::module*>((*lib_fun)(schedule));
  module_ = std::shared_ptr<motis::module::module>(ptr);
}

dynamic_module::~dynamic_module() {
  // Free the module itself.
  // This calls the destructor and must
  // therefore be done *before* freeing the shared library.
  module_ = nullptr;

  // Close shared library.
  if (lib_ != nullptr) {
    dlclose(lib_);
  }
}
#endif  // defined _WIN32 || defined _WIN64

}  // namespace motis
}  // namespace module
