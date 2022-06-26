#include "transport_catalogue.h"

using namespace domain;
using namespace transport_catalogue;
using namespace geo;
using namespace std;

void TransportCatalogue::AddStop(string_view stop, const Coordinates& coordinates) {
    
    string stop_name(stop);
    all_stops_.push_back({stop_name, coordinates});
    stopname_to_stop_[all_stops_.back().name] = &(all_stops_.back());
    stop_to_buses_[&(all_stops_.back())];
}

void TransportCatalogue::AddBus(string_view bus, const vector<string>& stops, bool is_roundtrip) {
    
    string bus_name(bus);
    all_buses_.push_back({bus_name, {}, is_roundtrip});
    all_buses_.back().stops.reserve(stops.size());
    for (const string& stop : stops) {
        all_buses_.back().stops.push_back(stopname_to_stop_[stop]);
    }
    
    BusPtr ptr_bus = &(all_buses_.back());
    
    busname_to_bus_[bus_name] = ptr_bus;
    
    for (const string& stop : stops) {
        StopPtr ptr_stop = stopname_to_stop_.at(stop);
        stop_to_buses_[ptr_stop].insert(ptr_bus);
    }
}

void TransportCatalogue::SetDistanceBetweenStops(string_view from, string_view to, int distance) {
    
    string from_name(from), to_name(to);
    StopPtr from_ptr = stopname_to_stop_.at(from_name);
    StopPtr to_ptr = stopname_to_stop_.at(to_name);
    distances_[{from_ptr, to_ptr}] = distance;
}

void TransportCatalogue::SetBusWaitTime(size_t bus_wait_time) {
    bus_wait_time_ = bus_wait_time;
}

void TransportCatalogue::SetBusVelocity(size_t bus_velocity) {
    bus_velocity_ = bus_velocity;
}

StopPtr TransportCatalogue::GetStop(string_view stop) const {
    
    string stop_name(stop);
    if (stopname_to_stop_.count(stop_name) == 0) {
        return nullptr;
    }
    return stopname_to_stop_.at(stop_name);
}

BusPtr TransportCatalogue::GetBus(string_view bus) const {
    
    string bus_name(bus);
    if (busname_to_bus_.count(bus_name) == 0) {
        return nullptr;
    }
    return busname_to_bus_.at(bus_name);
}

vector<BusPtr> TransportCatalogue::GetAllBuses() const {
    
    set<string> buses_sorted_by_names;
    std::for_each(all_buses_.begin(), all_buses_.end(), [&] (const Bus& bus) {
        buses_sorted_by_names.insert(bus.name);
    });
    vector<BusPtr> result(buses_sorted_by_names.size());
    std::transform(buses_sorted_by_names.begin(), buses_sorted_by_names.end(),
                   result.begin(), [*this] (const string& busname) {
        return GetBus(busname);
    });
    return result;
}

vector<StopPtr> TransportCatalogue::GetAllStops() const {
    
    vector<StopPtr> result;
    result.reserve(all_stops_.size());
    std::for_each(all_stops_.begin(), all_stops_.end(), [&] (const Stop& stop) {
        result.push_back(&stop);
    });
    return result;
}

vector<StopPtr> TransportCatalogue::GetAllNonEmptyStops() const {
    
    set<string> stops_sorted_by_name;
    std::for_each(all_stops_.begin(), all_stops_.end(), [&] (const Stop& stop) {
        stops_sorted_by_name.insert(stop.name);
    });
    vector<StopPtr> result;
    std::for_each(stops_sorted_by_name.begin(), stops_sorted_by_name.end(), [&] (const string& stopname) {
        if (!GetBusesForStop(stopname).empty()) {
           result.push_back(GetStop(stopname));
        }
    });
    return result;
}

pair<int, int> TransportCatalogue::GetWaitTimeAndVelocity() const {
    return {bus_wait_time_, bus_velocity_};
}


const unordered_set<BusPtr> TransportCatalogue::GetBusesForStop(string_view stop) const {
    
    static const unordered_set<const Bus*> empty_set;
    string stop_name(stop);
    if (stopname_to_stop_.count(stop_name) == 0) {
        return empty_set;
    }
    StopPtr ptr_stop = stopname_to_stop_.at(stop_name);
    return stop_to_buses_.at(ptr_stop);
}

int TransportCatalogue::GetDistanceBetweenStops(string_view from, string_view to) const {
    
    string from_name(from), to_name(to);
    StopPtr from_ptr = stopname_to_stop_.at(from_name);
    StopPtr to_ptr = stopname_to_stop_.at(to_name);
    if (distances_.count({from_ptr, to_ptr})) {
        return distances_.at({from_ptr, to_ptr});
    }
    assert(distances_.count({to_ptr, from_ptr}));
    return distances_.at({to_ptr, from_ptr});
}

bool TransportCatalogue::IsThereStop(string_view stop) const {
    return stopname_to_stop_.count(string(stop));
}

bool TransportCatalogue::IsThereBus(string_view bus) const {
    return busname_to_bus_.count(string(bus));
}


