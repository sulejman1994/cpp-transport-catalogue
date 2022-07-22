#pragma once

#include "geo.h"
#include "domain.h"
#include "transport_catalogue.pb.h"

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

using domain::Bus, domain::Stop;
using BusPtr = const domain::Bus*;
using StopPtr = const domain::Stop*;

class TransportCatalogue {
public:
    TransportCatalogue() = default;
    
    TransportCatalogue(const serialization::TransportCatalogue& tr_ser);
    
    void AddStop(std::string_view stop, const geo::Coordinates& coordinates);
    void AddBus(std::string_view bus, const std::vector<std::string>& stops, bool is_roundtrip);
    
    void SetDistanceBetweenStops(std::string_view from, std::string_view to, int distance);
    
    void SetBusWaitTime(size_t bus_wait_time);
    void SetBusVelocity(size_t bus_velocity);
    
    StopPtr GetStop(std::string_view stop) const;
    BusPtr GetBus(std::string_view bus) const;
    std::vector<BusPtr> GetAllBuses() const;
    std::vector<StopPtr> GetAllStops() const;
    std::vector<StopPtr> GetAllNonEmptyStops() const;
    std::pair<int, int> GetWaitTimeAndVelocity() const;
    const std::unordered_set<BusPtr> GetBusesForStop(std::string_view stop) const;
    int GetDistanceBetweenStops(std::string_view from, std::string_view to) const;
    
    bool IsThereStop(std::string_view stop) const;
    bool IsThereBus(std::string_view bus) const;
    
    struct Hasher {
        size_t operator() (const std::pair<StopPtr, StopPtr>& key) const {
            const int p = 37;
            return std::hash<StopPtr>() (key.first) + p * std::hash<StopPtr>() (key.second);
        }
    };
    
private:
    std::deque<Stop> all_stops_;
    std::deque<Bus> all_buses_;
    std::unordered_map<std::string, StopPtr> stopname_to_stop_;
    std::unordered_map<std::string, BusPtr> busname_to_bus_;
    std::unordered_map<StopPtr, std::unordered_set<BusPtr>> stop_to_buses_;
    std::unordered_map< std::pair<StopPtr, StopPtr>, int, Hasher> distances_;
    size_t bus_wait_time_ = 0;
    size_t bus_velocity_ = 40;
};

} // namespace transport_catalogue

