#include <iostream>

#include "motis/loader/loader.h"

#include "motis/railviz/railviz.h"
#define BOOST_TEST_MODULE RailvizProtocolTest
#include "boost/test/unit_test.hpp"

namespace motis {
namespace railviz {
namespace test {

struct Fixure
{
    Fixure()
    {
        auto schedule = motis::load_schedule("../schedule/test");
        r.schedule_ = schedule.get();
        r.init();
    }

    motis::railviz::railviz r;
};

BOOST_FIXTURE_TEST_SUITE( railviz_protocol_suit, Fixure )

BOOST_AUTO_TEST_CASE( railviz_protocol_test )
{
    std::cout << "hallo welt" << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
