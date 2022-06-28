#include "transport_router.h"

using namespace std;
using namespace graph;

namespace transport_catalogue {

TransportRouter::TransportRouter(const TransportCatalogue& transport_catalogue)
    : transport_catalogue_(transport_catalogue)
{
    
}

optional<graph::Router<double>::RouteInfo> TransportRouter::BuildRoute(string_view from, string_view to) const {
    if (!is_graph_built_) {
        BuildGraph();
        router_ = make_unique<Router<double>> (graph_);
        is_graph_built_ = true;
    }
    Vertex vertex_from{string(from), true}, vertex_to{string(to), true};
    return router_->BuildRoute(vertex_to_id_[vertex_from], vertex_to_id_[vertex_to]);
}

const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() const {
    return graph_;
}

const std::vector<TransportRouter::Vertex>& TransportRouter::GetIdToVertex() const {
    return id_to_vertex_;
}


void TransportRouter::BuildGraph() const {
    vector<StopPtr> all_stops(transport_catalogue_.GetAllStops());
    graph_ = DirectedWeightedGraph<double> (2 * all_stops.size());
    
    id_to_vertex_.reserve(2 * all_stops.size());
    AddVertexes(all_stops);
    
    vector<BusPtr> all_buses(transport_catalogue_.GetAllBuses());
    for (const BusPtr& bus : all_buses) { 
        AddEdgesForBus(bus);
    }
}
    
void TransportRouter::AddVertexes(const vector<StopPtr>& all_stops) const {
    for (const StopPtr& stop : all_stops) { 
        vertex_to_id_[{stop->name, true}] = id_to_vertex_.size();
        id_to_vertex_.push_back({stop->name, true});
        vertex_to_id_[{stop->name, false}] = id_to_vertex_.size();
        id_to_vertex_.push_back({stop->name, false});
    }
}

void TransportRouter::AddEdgesForBus(const BusPtr& bus) const {
    double wait_time = transport_catalogue_.GetWaitTimeAndVelocity().first;
    double velocity = (transport_catalogue_.GetWaitTimeAndVelocity().second) * 1000.0 / 60;
    
    vector<StopPtr> stops(bus->stops);
    
    for (size_t i = 0; i < stops.size(); ++i) {
        graph_.AddEdge({vertex_to_id_.at({stops[i]->name, true}), vertex_to_id_.at({stops[i]->name, false}), wait_time, bus->name, 0});
    }
    
    AddEdgesBetweenStops(stops, bus, velocity);
    
    if (bus->is_roundtrip) {
        return;
    }
    
    reverse(stops.begin(), stops.end());
    AddEdgesBetweenStops(stops, bus, velocity);
}

void TransportRouter::AddEdgesBetweenStops(const std::vector<StopPtr>& stops, const BusPtr& bus, double velocity) const {
    
    for (size_t i = 0; i < stops.size(); ++i) {
        double distance = 0;
        for (size_t j = i + 1; j < stops.size(); ++j) {
            distance += transport_catalogue_.GetDistanceBetweenStops(stops[j - 1]->name, stops[j]->name);
            graph_.AddEdge({vertex_to_id_.at({stops[i]->name, false}), vertex_to_id_.at({stops[j]->name, true}), distance / velocity, bus->name, j - i});
        }
    }
}

} // namespace transport_catalogue
