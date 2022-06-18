#pragma once

#include "geo.h"
#include "domain.h"

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
#include <algorithm>

namespace transport_catalogue {

using domain::Bus, domain::Stop, std::string, std::string_view, std::vector, std::deque, std::unordered_map, std::unordered_set, std::set, std::pair;

using BusPtr = const domain::Bus*;
using StopPtr = const domain::Stop*;

class TransportCatalogue {
public:
        
    void AddStop(string_view stop, const geo::Coordinates& coordinates);
    void AddBus(string_view bus, const vector<string>& stops, bool is_roundtrip);
    
    void SetDistanceBetweenStops(string_view from, string_view to, int distance);
    
    StopPtr GetStop(string_view stop) const;
    BusPtr GetBus(string_view bus) const;
    vector<BusPtr> GetAllBuses() const;
    vector<StopPtr> GetAllNonEmptyStops() const;
    const unordered_set<BusPtr> GetBusesForStop(string_view stop) const;
    int GetDistanceBetweenStops(string_view from, string_view to) const;
    
    bool IsThereStop(string_view stop) const;
    bool IsThereBus(string_view bus) const;
    
    struct Hasher {
        size_t operator() (const pair<StopPtr, StopPtr>& key) const {
            const int p = 37;
            return std::hash<StopPtr>() (key.first) + p * std::hash<StopPtr>() (key.second);
        }
    };
    
private:
    deque<Stop> all_stops_;
    deque<Bus> all_buses_;
    unordered_map<string, StopPtr> stopname_to_stop_;
    unordered_map<string, BusPtr> busname_to_bus_;
    unordered_map<StopPtr, unordered_set<BusPtr>> stop_to_buses_;
    unordered_map< pair<StopPtr, StopPtr>, int, Hasher> distances_;
};

} // namespace transport_catalogue
