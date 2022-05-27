#include "stat_reader.h"

#include <iostream>
#include <vector>
#include <unordered_set>
#include <set>
#include <iomanip>

using namespace global;
using transport_catalogue::Bus, transport_catalogue::Stop;
using std::vector, std::endl, std::setprecision, std::string, std::unordered_set, std::set;


void stat_reader::Output(istream& input, const TransportCatalogue& transport_catalogue, ostream& out) {
    
    int m;
    input >> m;
    while (input.peek() != '\n') {
        input.ignore(1);
    }
    for (int i = 0; i < m; ++i) {
        string operation_code;
        input >> operation_code;
        
        if (operation_code == "Bus") {
            OutputBus(input, transport_catalogue, out);
        }
        if (operation_code == "Stop") {
            OutputBusesForStop(input, transport_catalogue, out);
        }
    }
}

void stat_reader::OutputBus(istream& input, const TransportCatalogue& transport_catalogue, ostream& out) {
    
    string busname;
    while (input.peek() == ' ') {
        input.ignore(1);
    }
    getline(input, busname, '\n');
    while (busname.back() == ' ') {
        busname.pop_back();
    }
    out << "Bus " << busname << ": ";
    
    Bus bus = transport_catalogue.GetBus(busname);
    if (bus.stops.empty()) {
        out << "not found" << endl;
        return;
    }
    
    size_t route_size = bus.stops.size();
    
    unordered_set<const Stop*> set_unique_stops(bus.stops.begin(), bus.stops.end());
    size_t unique_stops = set_unique_stops.size();
    
    double geo_length = 0;
    int length = 0;
    
    for (int i = 1; i < route_size; ++i) {
        const Stop* prev_stop = bus.stops[i - 1];
        const Stop* cur_stop = bus.stops[i];
        geo_length += ComputeDistance(prev_stop->coordinates, cur_stop->coordinates);
        length += transport_catalogue.GetDistanceBetweenStops(prev_stop->stopname, cur_stop->stopname );
    }
    
    double curvature = length / geo_length;
    
    out << route_size << " stops on route, " << unique_stops << " unique stops, " << length << " route length, " << setprecision(6) << curvature << " curvature" << endl;
    
}

void stat_reader::OutputBusesForStop(istream& input, const TransportCatalogue& transport_catalogue, ostream& out) {
    
    while (input.peek() == ' ') {
        input.ignore(1);
    }
    string stopname;
    getline(input, stopname, '\n');
    while (stopname.back() == ' ') {
        stopname.pop_back();
    }
    
    out << "Stop " << stopname << ": ";
    
    if ( ! transport_catalogue.IsThereStop(stopname)) {
        out << "not found" << endl;
        return;
    }
    
    set<string> buses;
    
    for (const auto& ptr_bus : transport_catalogue.GetBusesForStop(stopname)) {
        buses.insert(ptr_bus->busname);
    }
    
    if (buses.empty()) {
        out << "no buses" << endl;
        return;
    }
    
    out << "buses ";
    for (const auto& bus : buses) {
        out << bus << " ";
    }
    out << endl;
}
