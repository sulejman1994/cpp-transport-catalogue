#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"

#include <unordered_set>
#include <set>

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

using transport_catalogue::TransportCatalogue, transport_catalogue::BusPtr, transport_catalogue::StopPtr, renderer::MapRenderer, std::string, std::unordered_set, std::set;

namespace request_handler {

class RequestHandler {
public:

    RequestHandler(const TransportCatalogue& db, const MapRenderer& renderer);

    BusPtr GetBus(string_view bus_name) const;
    
    vector<BusPtr> GetAllBuses() const;
    
    vector<StopPtr> GetAllNonEmptyStops() const;

    const unordered_set<BusPtr> GetBusesForStop(string_view stop_name) const;
    
    bool IsThereBus(string_view bus_name) const;
    
    bool IsThereStop(string_view stop_name) const;
    
    int GetDistanceBetweenStops(string_view from, string_view to) const;

    svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const TransportCatalogue& db_;
    const MapRenderer& renderer_;
};

} // namespace request_handler
