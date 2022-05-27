#include "input_reader.h"
#include "transport_catalogue.h"
#include "stat_reader.h"

#include <iostream>
#include <cassert>
#include <fstream>

using std::cin, std::cout;

namespace global {
    namespace geo {}
    namespace input_reader {}
    namespace transport_catalogue {}
    namespace stat_reader {}
}


int main() {
    
    global::transport_catalogue::TransportCatalogue transport_catalogue;
    global::input_reader::ReadInput(cin, transport_catalogue);
    global::stat_reader::Output(cin, transport_catalogue, cout);
    return 0;
}
