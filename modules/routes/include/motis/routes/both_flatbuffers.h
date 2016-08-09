#pragma once

#include "flatbuffers/flatbuffers.h"
#undef FLATBUFFERS_H_
#undef FLATBUFFERS_NAMESPACE
#define FLATBUFFERS_NAMESPACE flatbuffers64
#include "flatbuffers/flatbuffers.h"
