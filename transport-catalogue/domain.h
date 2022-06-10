#pragma once

#include "geo.h"

#include <vector>
#include <string>
using geo::Coordinates, std::vector, std::string;

namespace domain {
    
    struct Stop {
        string stopname;
        Coordinates coordinates;
    };

    struct Bus {
        string busname;
        vector<const Stop*> stops;
        bool is_roundtrip = false;
    };

} // namespace domain
