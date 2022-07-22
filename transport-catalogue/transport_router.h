#pragma once

#include "router.h"
#include "transport_catalogue.h"
#include "transport_catalogue.pb.h"

#include <memory>

namespace transport_catalogue {

class TransportRouter {
public:
    TransportRouter(const transport_catalogue::TransportCatalogue& transport_catalogue);
    
    TransportRouter(const serialization::TransportCatalogue& transport_catalogue, const transport_catalogue::TransportCatalogue& transport_catalogue_usual);
    
    std::optional<graph::Router<double>::RouteInfo> BuildRoute(std::string_view from, std::string_view to) const;
    
    struct Vertex {
        std::string stopname;
        bool is_waiting;
        
        bool operator < (const Vertex& other) const {
            return std::pair{stopname, is_waiting} < std::pair{other.stopname, other.is_waiting};
        }
    };
    
    void BuildGraph() const;
    
    void BuildRouter() const;
    
    const graph::DirectedWeightedGraph<double>& GetGraph() const;
    
    const graph::Router<double>* GetRouter() const;
    
    const std::vector<Vertex>& GetIdToVertex() const;
    
private:
    const TransportCatalogue& transport_catalogue_;
    mutable std::unique_ptr<graph::Router<double>> router_;
    mutable graph::DirectedWeightedGraph<double> graph_;
    mutable std::map<Vertex, size_t> vertex_to_id_;
    mutable std::vector<Vertex> id_to_vertex_;
    mutable bool is_graph_built_ = false;
    mutable bool is_router_built_ = false;
    
    void AddVertexes(const std::vector<StopPtr>& all_stops) const;
    
    void AddEdgesForBus(const BusPtr& bus) const;
    
    void AddEdgesBetweenStops(const std::vector<StopPtr>& stops, const BusPtr& bus, double velocity) const;
    
    void SetIdToVertex(const serialization::TransportCatalogue& transport_catalogue) const;
    
    void SetVertexToId() const;
    
    graph::DirectedWeightedGraph<double> SetGraph(const serialization::Graph& graph) const;
    
    std::unique_ptr<graph::Router<double>> SetRouter(const serialization::Router& router) const;
    
};
    
} // namespace transport_catalogue

