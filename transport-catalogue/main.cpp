#include "transport_catalogue.h"
#include "json_reader.h"
#include "request_handler.h"
#include "map_renderer.h"

#include <iostream>
#include <cassert>
#include <fstream>
#include <ctime>

using std::cin, std::cout, std::ifstream, std::ofstream;

namespace domain {}
namespace geo {}
namespace transport_catalogue {}
namespace json {}
namespace json_reader {}
namespace request_handler {}
namespace svg {}
namespace renderer {}


int main() {
    
    ifstream input("input.txt");
    ofstream output("output.txt");
    transport_catalogue::TransportCatalogue transport_catalogue;
    json_reader::ReadInputAndProcessRequests(input, transport_catalogue, output);
    
    return 0;
}

