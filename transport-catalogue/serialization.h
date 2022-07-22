#pragma once

#include "json.h"
#include "transport_catalogue.pb.h"
#include "transport_router.h"

#include <iostream>

namespace serialization {

class Serializer {
public:
    
    void SerializeFromInput(std::istream& input);
   
private:
    json::Document requests_;
    std::vector<json::Node> base_requests_;
    TransportCatalogue transport_catalogue_;
    std::unordered_map<std::string, size_t> stopname_to_index_;
    
    void SerializeStops();
    void SerializeDistancesAndBuses();
    
    void SerializeRenderSettings();
    static Color ReadColor(const json::Node& color_node);
    
    void SerializeRoutingSettings();
    void SerializeRouter();
    static Graph SerializeGraph(const graph::DirectedWeightedGraph<double>& graph_usual);
    static Edge SerializeEdge(const graph::Edge<double>& edge_usual);
    static Router SerializeRouteInternalData(const transport_catalogue::TransportRouter& router_usual);
    
    using VertexUsual = transport_catalogue::TransportRouter::Vertex;
    void SerializeIdToVertex(const std::vector<VertexUsual>& id_to_vertex);

    template <typename IteratorRange>
    static IncidenceList SerializeIncidenceList(IteratorRange range);
    
    void SerializeToOstream() const;
};

template <typename IncidenceListRange>
IncidenceList Serializer::SerializeIncidenceList(IncidenceListRange range) {
    IncidenceList result;
    for (auto it = range.begin(); it != range.end(); ++it) {
        result.add_edge_id(*it);
    }
    return result;
}




void Serialize(std::istream& input);

RenderSettings SerializeRenderSettings(const std::map<std::string, json::Node>& settings);
Color ReadColor(const json::Node& color_node);

void SerializeRouter(TransportCatalogue& tr);
Graph SerializeGraph(const graph::DirectedWeightedGraph<double>& graph);
Edge SerializeEdge(const graph::Edge<double>& edge_usual);

template <typename IteratorRange>
IncidenceList SerializeIncidenceList(IteratorRange range);


using VertexUsual = transport_catalogue::TransportRouter::Vertex;

void SerializeIdToVertex(TransportCatalogue& tr, const std::vector<VertexUsual>& id_to_vertex);
Router SerializeRouteInternalData(const transport_catalogue::TransportRouter& router_usual);



template <typename IncidenceListRange>
IncidenceList SerializeIncidenceList(IncidenceListRange range) {
    IncidenceList result;
    for (auto it = range.begin(); it != range.end(); ++it) {
        result.add_edge_id(*it);
    }
    return result;
}

}

