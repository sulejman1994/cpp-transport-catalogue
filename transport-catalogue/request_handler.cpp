#include "request_handler.h"

using namespace request_handler;

RequestHandler::RequestHandler(const TransportCatalogue& db, const MapRenderer& renderer)
    : db_(db), renderer_(renderer) {
}

const Bus* RequestHandler::GetBus(string_view bus_name) const {
    return db_.GetBus(bus_name);
}

vector<const Bus*> RequestHandler::GetAllBuses() const {
    return db_.GetAllBuses();
}

vector<const Stop*> RequestHandler::GetAllNonEmptyStops() const {
    return db_.GetAllNonEmptyStops();
}

const unordered_set<const Bus*> RequestHandler::GetBusesForStop(string_view stop_name) const {
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
