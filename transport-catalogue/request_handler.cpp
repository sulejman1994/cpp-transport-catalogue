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
