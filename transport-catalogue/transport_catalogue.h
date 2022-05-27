#pragma once

#include "geo.h"

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <cassert>

using std::string, std::string_view, std::vector, std::deque, std::unordered_map, std::unordered_set, std::cout, std::endl, std::pair, std::map, std::hash;
using namespace global::geo;

namespace global::transport_catalogue {

struct Stop {
    string stopname;
    Coordinates coordinates;
};

struct Bus {
    string busname;
    vector<const Stop*> stops;
};

class TransportCatalogue {
public:
    
    void AddStop(string_view stop, Coordinates coordinates);
    
    void AddBus(string_view bus, const vector<string>& stops);
    
    void SetDistanceBetweenStops(string_view from, string_view to, int distance);
    
    const Stop& GetStop(string_view stop) const;
    
    const Bus& GetBus(string_view bus) const;
    
    const unordered_set<const Bus*> GetBusesForStop(string_view stop) const;
    
    int GetDistanceBetweenStops(string_view from, string_view to) const;
    
    bool IsThereStop(string_view stop) const;
    
    bool IsThereBus(string_view bus) const;
    
    struct Hasher {
        size_t operator() (const pair<const Stop*, const Stop*>& key) const {
            const int p = 37;
            return hash<const Stop*>() (key.first) + p * hash<const Stop*>() (key.second);
        }
    };
    
private:
    deque<Stop> all_stops_;
    deque<Bus> all_buses_;
    unordered_map<string, const Stop*> stopname_to_stop_;
    unordered_map<string, const Bus*> busname_to_bus_;
    unordered_map<const Stop*, unordered_set<const Bus*>> stop_to_buses_;
    unordered_map< pair<const Stop*, const Stop*>, int, Hasher> distances_;
    
};

}
