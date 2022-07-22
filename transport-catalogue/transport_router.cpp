#include "transport_router.h"

#include <optional>
#include <cmath>
#include <iostream>

using namespace std;
using namespace graph;

namespace transport_catalogue {

TransportRouter::TransportRouter(const TransportCatalogue& transport_catalogue)
    : transport_catalogue_(transport_catalogue)
{
    
}


TransportRouter::TransportRouter(const serialization::TransportCatalogue& transport_catalogue, const TransportCatalogue& transport_catalogue_usual)
    : transport_catalogue_(transport_catalogue_usual),
      graph_(SetGraph(transport_catalogue.router().graph())),
      router_(SetRouter(transport_catalogue.router()))
{
    SetIdToVertex(transport_catalogue);
    SetVertexToId();
    is_graph_built_ = true;
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

const graph::Router<double>* TransportRouter::GetRouter() const {
    return router_.get();
}


void TransportRouter::BuildGraph() const {
    if (is_graph_built_) {
        return;
    }
    vector<StopPtr> all_stops(transport_catalogue_.GetAllStops());
    graph_ = DirectedWeightedGraph<double> (2 * all_stops.size());
    
    id_to_vertex_.reserve(2 * all_stops.size());
    AddVertexes(all_stops);
    
    vector<BusPtr> all_buses(transport_catalogue_.GetAllBuses());
    for (const BusPtr& bus : all_buses) {
        AddEdgesForBus(bus);
    }
    is_graph_built_ = true;
}

void TransportRouter::BuildRouter() const {
    if (is_router_built_) {
        return;
    }
    if (!is_graph_built_) {
        BuildGraph();
        is_graph_built_ = true;
    }
    router_ = make_unique<Router<double>> (graph_);
    is_router_built_ = true;
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


void TransportRouter::SetIdToVertex(const serialization::TransportCatalogue& transport_catalogue) const {
    for (size_t i = 0; i < transport_catalogue.id_to_vertex_size(); ++i) {
        id_to_vertex_.push_back({transport_catalogue.id_to_vertex(i).stopname(), transport_catalogue.id_to_vertex(i).is_waiting()});
    }
}

void TransportRouter::SetVertexToId() const {
    for (size_t i = 0; i < id_to_vertex_.size(); ++i) {
        vertex_to_id_[id_to_vertex_[i]] = i;
    }
}

graph::DirectedWeightedGraph<double> TransportRouter::SetGraph(const serialization::Graph& graph) const {
    vector<graph::Edge<double>> edges;
    for (size_t i = 0; i < graph.edge_size(); ++i) {
        graph::Edge<double> edge_usual;
        edge_usual.from = graph.edge(i).from();
        edge_usual.to = graph.edge(i).to();
        edge_usual.weight = graph.edge(i).weight();
        edge_usual.busname = graph.edge(i).busname();
        edge_usual.stop_count = graph.edge(i).stop_count();
        edges.push_back(edge_usual);
    }
    
    vector<vector<size_t>> incidence_lists;
    for (size_t i = 0; i < graph.incidence_list_size(); ++i) {
        vector<size_t> incidence_list_tmp;
        for (size_t k = 0; k < graph.incidence_list(i).edge_id_size(); ++k) {
            incidence_list_tmp.push_back(graph.incidence_list(i).edge_id(k));
        }
        incidence_lists.push_back(incidence_list_tmp);
    }
    return graph::DirectedWeightedGraph<double> (edges, incidence_lists);
}

unique_ptr<graph::Router<double>> TransportRouter::SetRouter(const serialization::Router& router) const {
    
    using OptionalRouteInternalData = optional<graph::Router<double>::RouteInternalData>;
    vector<vector<OptionalRouteInternalData>> route_internal_data;
    
    for (size_t i = 0; i < router.route_internal_data_list_size(); ++i) {
        vector<OptionalRouteInternalData> route_internal_data_list_tmp;
        for (size_t k = 0; k < router.route_internal_data_list(i).route_internal_data_size(); ++k) {
            OptionalRouteInternalData tmp = make_optional<graph::Router<double>::RouteInternalData> ();
            tmp.value().weight = router.route_internal_data_list(i).route_internal_data(k).weight();
            tmp.value().prev_edge = make_optional<size_t> (-1);
            if (tmp->weight < 0) {
                route_internal_data_list_tmp.push_back(nullopt);
                continue;
            }
            if (router.route_internal_data_list(i).route_internal_data(k).is_prev_edge()) {
                tmp->prev_edge = make_optional<size_t> (router.route_internal_data_list(i).route_internal_data(k).prev_edge());
            } else {
                tmp->prev_edge = nullopt;
            }
            route_internal_data_list_tmp.push_back(tmp);
        }
        route_internal_data.push_back(route_internal_data_list_tmp);
    }
   return make_unique<graph::Router<double>> (graph_, route_internal_data);
}


} // namespace transport_catalogue

