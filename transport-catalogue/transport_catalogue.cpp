#include "transport_catalogue.h"

using namespace global;
using transport_catalogue::Bus, transport_catalogue::Stop;

void transport_catalogue::TransportCatalogue::AddStop(string_view stop, Coordinates coordinates) {
    
    string stop_name(stop);
    all_stops_.push_back({stop_name, coordinates});
    stopname_to_stop_[all_stops_.back().stopname] = &(all_stops_.back());
    stop_to_buses_[&(all_stops_.back())];
}

void transport_catalogue::TransportCatalogue::AddBus(string_view bus, const vector<string>& stops) {
    
    string bus_name(bus);
    all_buses_.push_back({bus_name, {}});
    all_buses_.back().stops.reserve(stops.size());
    for (const string& stop : stops) {
        all_buses_.back().stops.push_back(stopname_to_stop_[stop]);
    }
    
    const Bus* ptr_bus = &(all_buses_.back());
    
    busname_to_bus_[bus_name] = ptr_bus;
    
    for (const string& stop : stops) {
        const Stop* ptr_stop = stopname_to_stop_.at(stop);
        stop_to_buses_[ptr_stop].insert(ptr_bus);
    }
}

void transport_catalogue::TransportCatalogue::SetDistanceBetweenStops(string_view from, string_view to, int distance) {
    
    string from_name(from), to_name(to);
    const Stop* from_ptr = stopname_to_stop_.at(from_name);
    const Stop* to_ptr = stopname_to_stop_.at(to_name);
    distances_[{from_ptr, to_ptr}] = distance;
}

const Stop& global::transport_catalogue::TransportCatalogue::GetStop(string_view stop) const {
    
    static Stop empty_stop;
    string stop_name(stop);
    if (stopname_to_stop_.count(stop_name) == 0) {
        return empty_stop;
    }
    return *(stopname_to_stop_.at(stop_name));
}

const Bus& transport_catalogue::TransportCatalogue::GetBus(string_view bus) const {
    
    static const Bus empty_bus;
    string bus_name(bus);
    
    if (busname_to_bus_.count(bus_name) == 0) {
        return empty_bus;
    }
    return *(busname_to_bus_.at(bus_name));
}

const unordered_set<const Bus*> transport_catalogue::TransportCatalogue::GetBusesForStop(string_view stop) const {
    
    static const unordered_set<const Bus*> empty_set;
    string stop_name(stop);
    if (stopname_to_stop_.count(stop_name) == 0) {
        return empty_set;
    }
    const Stop* ptr_stop = stopname_to_stop_.at(stop_name);
    return stop_to_buses_.at(ptr_stop);
}

int transport_catalogue::TransportCatalogue::GetDistanceBetweenStops(string_view from, string_view to) const {
    
    string from_name(from), to_name(to);
    const Stop* from_ptr = stopname_to_stop_.at(from_name);
    const Stop* to_ptr = stopname_to_stop_.at(to_name);
    if (distances_.count({from_ptr, to_ptr})) {
        return distances_.at({from_ptr, to_ptr});
    }
    assert(distances_.count({to_ptr, from_ptr}));
    return distances_.at({to_ptr, from_ptr});
}

bool transport_catalogue::TransportCatalogue::IsThereStop(string_view stop) const {
    return stopname_to_stop_.count(string(stop));
}

bool transport_catalogue::TransportCatalogue::IsThereBus(string_view bus) const {
    return busname_to_bus_.count(string(bus));
}


