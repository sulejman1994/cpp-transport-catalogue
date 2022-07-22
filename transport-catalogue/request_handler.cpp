#include "request_handler.h"

using namespace std;
using namespace request_handler;
using namespace transport_catalogue;

RequestHandler::RequestHandler(const TransportCatalogue& db, const MapRenderer& renderer, const TransportRouter& router)
    : db_(db), renderer_(renderer), router_(router) {
}

std::optional<graph::Router<double>::RouteInfo> RequestHandler::BuildRoute(string_view from, string_view to) const {
    return router_.BuildRoute(from, to);
}

const TransportRouter& RequestHandler::GetRouter() const {
    return router_;
}

BusPtr RequestHandler::GetBus(string_view bus_name) const {
    return db_.GetBus(bus_name);
}

vector<BusPtr> RequestHandler::GetAllBuses() const {
    return db_.GetAllBuses();
}

vector<StopPtr> RequestHandler::GetAllStops() const {
    return db_.GetAllStops();
}

vector<StopPtr> RequestHandler::GetAllNonEmptyStops() const {
    return db_.GetAllNonEmptyStops();
}

pair<int, int> RequestHandler::GetWaitTimeAndVelocity() const {
    return db_.GetWaitTimeAndVelocity();
}

const unordered_set<BusPtr> RequestHandler::GetBusesForStop(string_view stop_name) const {
    return db_.GetBusesForStop(stop_name);
}

bool RequestHandler::IsThereBus(string_view bus_name) const {
    return db_.IsThereBus(bus_name);
}

bool RequestHandler::IsThereStop(string_view stop_name) const {
    return db_.IsThereStop(stop_name);
}

int RequestHandler::GetDistanceBetweenStops(string_view from, string_view to) const {
    return db_.GetDistanceBetweenStops(from, to);
}

svg::Document RequestHandler::RenderMap() const {
    return renderer_.Render(GetAllBuses(), GetAllNonEmptyStops());
}

optional<vector<string>> RequestHandler::ProcessStopRequest(const string& stopname) const {
    if (!IsThereStop(stopname)) {
        return nullopt;
    }
    set<string> buses;
    for (const Bus* bus : GetBusesForStop(stopname)) {
        buses.insert(bus->name);
    }
    return {{buses.begin(), buses.end()}};
}

optional<BusRequestResult> RequestHandler::ProcessBusRequest(const std::string& busname) const {
    if (!IsThereBus(busname)) {
        return nullopt;
    }
    const Bus* bus = GetBus(busname);
    size_t stop_count = bus->stops.size();
    
    size_t length = 0;
    double geo_length = 0;
        
    for (int i = 1; i < stop_count; ++i) {
        const Stop* prev_stop = bus->stops[i - 1];
        const Stop* cur_stop = bus->stops[i];
        length += GetDistanceBetweenStops(prev_stop->name, cur_stop->name);
        geo_length += ComputeDistance(prev_stop->coordinates, cur_stop->coordinates);
        if (!bus->is_roundtrip) {
            length += GetDistanceBetweenStops(cur_stop->name, prev_stop->name);
        }
    }
    
    if (!bus->is_roundtrip) {
        stop_count = stop_count * 2 - 1;
        geo_length *= 2;
    }
    
    double curvature = length * 1.0 / geo_length;
    unordered_set<const Stop*> unique_stops({bus->stops.begin(), bus->stops.end()});
    size_t unique_stops_count = unique_stops.size();
    return {{length, stop_count, curvature, unique_stops_count}};
}

