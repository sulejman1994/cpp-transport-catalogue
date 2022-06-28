#include "transport_catalogue.h"
#include "json_reader.h"
#include "request_handler.h"
#include "map_renderer.h"

#include <iostream>
#include <cassert>
#include <fstream>
#include <ctime>

using namespace std;

namespace domain {}
namespace geo {}
namespace transport_catalogue {}
namespace json {}
namespace json_reader {}
namespace request_handler {}
namespace svg {}
namespace renderer {}
namespace graph {}
namespace ranges {}


int main() {
    
    ifstream input("input.txt");
    ofstream output("output.txt");
    transport_catalogue::TransportCatalogue transport_catalogue;
    json_reader::ReadInputAndProcessRequests(cin, transport_catalogue, cout);
   
    return 0;
}

