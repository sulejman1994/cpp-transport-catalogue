#pragma once

#include "transport_catalogue.h"

#include <iostream>
#include <vector>

using std::cout, std::istream, std::ostream;
using global::transport_catalogue::TransportCatalogue;

namespace global::stat_reader {

void Output(istream& input, const TransportCatalogue& transport_catalogue, ostream& out = cout);

void OutputBus(istream& input, const TransportCatalogue& transport_catalogue, ostream& out = cout);

void OutputBusesForStop(istream& input, const TransportCatalogue& transport_catalogue, ostream& out = cout);

}
