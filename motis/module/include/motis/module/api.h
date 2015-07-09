#pragma once

#include "motis/module/module.h"
#include "motis/module/handler_functions.h"

#ifndef MOTIS_STATIC_MODULES

#if defined _WIN32 || defined _WIN64
#define MOTIS_EXP_FUNCTION __declspec(dllexport)
#define MOTIS_CALLING_CONVENTION __cdecl
#else
#define MOTIS_EXP_FUNCTION extern "C"
#define MOTIS_CALLING_CONVENTION
#endif

extern "C" {

MOTIS_EXP_FUNCTION void* MOTIS_CALLING_CONVENTION
    load_module(void*, void*, void*);

}  // extern "C"

#define MOTIS_MODULE_DEF_MODULE(name)                                   \
  extern "C" {                                                          \
  MOTIS_EXP_FUNCTION void* MOTIS_CALLING_CONVENTION                     \
      load_module(void* schedule, void* send, void* dispatch) {         \
    auto m = new motis::name::name();                                   \
    m->schedule_ = static_cast<motis::schedule*>(schedule);             \
    m->send_ = static_cast<motis::module::send_fun*>(send);             \
    m->dispatch_ = static_cast<motis::module::dispatch_fun*>(dispatch); \
    m->init();                                                          \
    return m;                                                           \
  }                                                                     \
  }

#else  // MOTIS_STATIC_MODULES

#define MOTIS_MODULE_DEF_MODULE(name)

#endif  // MOTIS_STATIC_MODULES
