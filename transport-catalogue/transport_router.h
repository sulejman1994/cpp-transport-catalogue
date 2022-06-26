#pragma once

#include "router.h"
#include "transport_catalogue.h"

namespace transport_catalogue {

class TransportRouter {
public:
    TransportRouter(const TransportCatalogue& transport_catalogue);
    
    std::optional<graph::Router<double>::RouteInfo> BuildRoute(std::string_view from, std::string_view to) const;
    
    struct Vertex {
        std::string stopname;
        bool is_waiting;
        
        bool operator < (const Vertex& other) const {
            return std::pair{stopname, is_waiting} < std::pair{other.stopname, other.is_waiting};
        }
    };
    
    const graph::DirectedWeightedGraph<double>& GetGraph() const;
    
    const std::vector<Vertex>& GetIdToVertex() const;
    
    TransportRouter(const TransportRouter&) = delete;
    TransportRouter& operator = (const TransportRouter&) = delete;
    TransportRouter(TransportRouter&&) = delete;
    TransportRouter& operator = (TransportRouter&&) = delete;

    ~TransportRouter() {
        if (router_) {
            delete router_;
        }
    }
    
private:
    const TransportCatalogue& transport_catalogue_;
    mutable graph::Router<double>* router_;
    mutable graph::DirectedWeightedGraph<double> graph_;
    mutable std::map<Vertex, size_t> vertex_to_id_;
    mutable std::vector<Vertex> id_to_vertex_;
    mutable bool is_graph_built_ = false;
    
    void BuildGraph() const;
    
    void AddEdgesForBus(const BusPtr& bus) const;
    
    void AddEdgesBetweenStops(const std::vector<StopPtr>& stops, const BusPtr& bus, double wait_time, double velocity) const;
};
    
} // namespace transport_catalogue
