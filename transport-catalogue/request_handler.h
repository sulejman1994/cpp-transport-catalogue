#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <unordered_set>
#include <set>

using transport_catalogue::TransportCatalogue, transport_catalogue::TransportRouter, transport_catalogue::BusPtr, transport_catalogue::StopPtr, renderer::MapRenderer;

namespace request_handler {

struct BusRequestResult {
    size_t route_length;
    size_t stop_count;
    double curvature;
    size_t unique_stop_count;
};

class RequestHandler {
public:

    RequestHandler(const TransportCatalogue& db, const MapRenderer& renderer, const TransportRouter& router);
    
    std::optional<std::vector<std::string>> ProcessStopRequest(const std::string& stopname) const;
    std::optional<BusRequestResult> ProcessBusRequest(const std::string& busname) const;
    
    std::optional<graph::Router<double>::RouteInfo> BuildRoute(std::string_view from, std::string_view to) const;
    const TransportRouter& GetRouter() const;

    BusPtr GetBus(std::string_view bus_name) const;
    std::vector<BusPtr> GetAllBuses() const;
    std::vector<StopPtr> GetAllStops() const;
    std::vector<StopPtr> GetAllNonEmptyStops() const;
    std::pair<int, int> GetWaitTimeAndVelocity() const;
    const std::unordered_set<BusPtr> GetBusesForStop(std::string_view stop_name) const;
    
    bool IsThereBus(std::string_view bus_name) const;
    bool IsThereStop(std::string_view stop_name) const;
    
    int GetDistanceBetweenStops(std::string_view from, std::string_view to) const;

    svg::Document RenderMap() const;
    
private:
    const TransportCatalogue& db_;
    const MapRenderer& renderer_;
    const TransportRouter& router_;
};

} // namespace request_handler

