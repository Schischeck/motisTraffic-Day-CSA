#pragma once

#include <memory>
#include <ctime>

namespace motis {
namespace railviz {

struct train
{
    std::time_t d_time;
    std::time_t a_time;
    unsigned int d_station;
    unsigned int a_station;
};

typedef std::unique_ptr<train> train_ptr;

}
}
