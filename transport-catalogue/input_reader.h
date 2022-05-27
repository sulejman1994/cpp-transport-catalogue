#pragma once

#include "geo.h"
#include "transport_catalogue.h"

#include <iostream>
#include <vector>

using std::istream, std::istringstream, std::string, std::vector;
using global::transport_catalogue::TransportCatalogue;

namespace global::input_reader {

void ReadInput(istream& input, TransportCatalogue& transport_catalogue);

void CollectAndSortQueries(istream& input, vector<string>& queries);

void ReadStopWithoutDistances(istringstream& stream, TransportCatalogue& transport_catalogue);

void ReadStopDistances(istringstream& stream, TransportCatalogue& transport_catalogue);

void ReadBus(istringstream& stream, TransportCatalogue& transport_catalogue);

}
