#pragma once

#include "geo.h"

#include <vector>
#include <string>

namespace domain {

    struct Stop {
        std::string name;
        geo::Coordinates coordinates;
    };

    struct Bus {
        std::string name;
        std::vector<const Stop*> stops;
        bool is_roundtrip = false;
    };

} // namespace domain
