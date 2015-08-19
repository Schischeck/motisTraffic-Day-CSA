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

MOTIS_EXP_FUNCTION void* MOTIS_CALLING_CONVENTION load_module(void*);

}  // extern "C"

#define MOTIS_MODULE_DEF_MODULE(name)                        \
  extern "C" {                                               \
  MOTIS_EXP_FUNCTION void* MOTIS_CALLING_CONVENTION          \
  load_module(void* context) {                               \
    auto m = new motis::name::name();                        \
    m->init_(static_cast<motis::module::context*>(context)); \
    return m;                                                \
  }                                                          \
  }

#else  // MOTIS_STATIC_MODULES

#define MOTIS_MODULE_DEF_MODULE(name)

#endif  // MOTIS_STATIC_MODULES
