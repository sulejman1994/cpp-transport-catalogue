#include "serialization.h"

#include <fstream>
using namespace std;
using namespace json;

namespace serialization {

void Serializer::SerializeFromInput(istream& input) {
    requests_ = json::Load(input);
    base_requests_ = requests_.GetRoot().AsMap().at("base_requests").AsArray();
    
    SerializeStops();
    SerializeDistancesAndBuses();
    SerializeRoutingSettings();
    SerializeRouter();
    SerializeRenderSettings();
    SerializeToOstream();
}

void Serializer::SerializeStops() {
    size_t stop_counter = 0;
    for (const Node& node : base_requests_) {
        if (node.AsMap().at("type").AsString() != "Stop") {
            continue;
        }
        map<string, Node> stop = node.AsMap();
        stopname_to_index_[stop.at("name").AsString()] = stop_counter++;
        Stop stop_serialized;
        *(stop_serialized.mutable_name()) = (stop.at("name").AsString());
        stop_serialized.mutable_coordinates()->set_lat(stop.at("latitude").AsDouble());
        stop_serialized.mutable_coordinates()->set_lng(stop.at("longitude").AsDouble());
        *(transport_catalogue_.add_stop()) = stop_serialized;
    }
}

void Serializer::SerializeDistancesAndBuses() {
    for (const Node& node : base_requests_) {
        if (node.AsMap().at("type").AsString() == "Stop") {
            map<string, Node> stop = node.AsMap();
            string stop_from = stop.at("name").AsString();
            FromToDistance tmp_from_to_dist;
            tmp_from_to_dist.set_from(stopname_to_index_[stop_from]);
            for (const auto& [stop_to, distance] : stop.at("road_distances").AsMap()) {
                tmp_from_to_dist.set_to(stopname_to_index_[stop_to]);
                tmp_from_to_dist.set_distance(distance.AsInt());
                *(transport_catalogue_.add_from_to_distance()) = tmp_from_to_dist;
            }
        }
        
        if (node.AsMap().at("type").AsString() == "Bus") {
            Bus tmp_bus;
            tmp_bus.set_is_roundtrip(node.AsMap().at("is_roundtrip").AsBool());
            tmp_bus.set_name(node.AsMap().at("name").AsString());
            for (const Node& stopname : node.AsMap().at("stops").AsArray()) {
                tmp_bus.add_stop_index(stopname_to_index_[stopname.AsString()]);
            }
            *(transport_catalogue_.add_bus()) = tmp_bus;
        }
    }
}

void Serializer::SerializeRoutingSettings() {
    transport_catalogue_.set_bus_wait_time(requests_.GetRoot().AsMap().at("routing_settings").AsMap().at("bus_wait_time").AsInt());
    transport_catalogue_.set_bus_velocity(requests_.GetRoot().AsMap().at("routing_settings").AsMap().at("bus_velocity").AsInt());
}

void Serializer::SerializeRenderSettings() {
    auto settings = requests_.GetRoot().AsMap().at("render_settings").AsMap();
    RenderSettings render_settings;
    
    render_settings.set_width(settings.at("width").AsDouble());
    render_settings.set_height(settings.at("height").AsDouble());
    render_settings.set_padding(settings.at("padding").AsDouble());
    render_settings.set_line_width(settings.at("line_width").AsDouble());
    render_settings.set_stop_radius(settings.at("stop_radius").AsDouble());
    render_settings.set_bus_label_font_size(settings.at("bus_label_font_size").AsInt());
    render_settings.set_stop_label_font_size(settings.at("stop_label_font_size").AsInt());
    render_settings.set_underlayer_width(settings.at("underlayer_width").AsDouble());
    
    render_settings.mutable_bus_label_offset()->set_x(settings.at("bus_label_offset").AsArray()[0].AsDouble());
    render_settings.mutable_bus_label_offset()->set_y(settings.at("bus_label_offset").AsArray()[1].AsDouble());
    render_settings.mutable_stop_label_offset()->set_x(settings.at("stop_label_offset").AsArray()[0].AsDouble());
    render_settings.mutable_stop_label_offset()->set_y(settings.at("stop_label_offset").AsArray()[1].AsDouble());
    
    *(render_settings.mutable_underlayer_color()) = ReadColor(settings.at("underlayer_color"));
    
    for (const Node& color_node : settings.at("color_palette").AsArray()) {
        render_settings.mutable_color_palette()->Add(ReadColor(color_node));
    }
    
    *(transport_catalogue_.mutable_render_settings()) = render_settings;
}

Color Serializer::ReadColor(const json::Node& color_node) {
    Color result;
    if (color_node.IsString()) {
        result.set_color_string(color_node.AsString());
        return result;
    }
    int red, green, blue;
    red = color_node.AsArray()[0].AsInt();
    green = color_node.AsArray()[1].AsInt();
    blue = color_node.AsArray()[2].AsInt();
    result.add_color_rgb(red);
    result.add_color_rgb(green);
    result.add_color_rgb(blue);
    if (color_node.AsArray().size() == 3) {
        result.set_is_rgb(true);
        return result;
    }
    result.set_opasity(color_node.AsArray()[3].AsDouble());
    result.set_is_rgb(false);
    return result;
}

void Serializer::SerializeRouter() {
    transport_catalogue::TransportCatalogue transport_catalogue_usual(transport_catalogue_);
    transport_catalogue::TransportRouter router_usual(transport_catalogue_usual);
    router_usual.BuildGraph();
    router_usual.BuildRouter();
    auto graph_usual = router_usual.GetGraph();
    
    SerializeIdToVertex(router_usual.GetIdToVertex());
    
    Router router = SerializeRouteInternalData(router_usual);;
    *(router.mutable_graph()) = SerializeGraph(graph_usual);
    
    *(transport_catalogue_.mutable_router()) = router;
}

void Serializer::SerializeIdToVertex(const std::vector<VertexUsual>& id_to_vertex) {
    for (const auto& vertex_usual : id_to_vertex) {
        Vertex vertex;
        vertex.set_stopname(vertex_usual.stopname);
        vertex.set_is_waiting(vertex_usual.is_waiting);
        *(transport_catalogue_.add_id_to_vertex()) = vertex;
    }
}

Graph Serializer::SerializeGraph(const graph::DirectedWeightedGraph<double>& graph_usual) {
    Graph result;
    for (size_t i = 0; i < graph_usual.GetEdgeCount(); ++i) {
        *(result.add_edge()) = SerializeEdge(graph_usual.GetEdge(i));
    }
    
    for (size_t i = 0; i < graph_usual.GetVertexCount(); ++i) {
        *(result.add_incidence_list()) = SerializeIncidenceList(graph_usual.GetIncidentEdges(i));
    }
    return result;
}

Edge Serializer::SerializeEdge(const graph::Edge<double>& edge_usual) {
    Edge result;
    result.set_from(edge_usual.from);
    result.set_to(edge_usual.to);
    result.set_weight(edge_usual.weight);
    result.set_stop_count(edge_usual.stop_count);
    result.set_busname(edge_usual.busname);
    return result;
}

Router Serializer::SerializeRouteInternalData(const transport_catalogue::TransportRouter& router_usual) {
    Router result;
    auto* router_usual_ptr = router_usual.GetRouter();
    
    for(const auto& router_internal_data_list_usual : router_usual_ptr->GetRoutesInternalData()) {
        
        RouteInternalDataList router_internal_data_list;
        for (const auto& internal_data_usual : router_internal_data_list_usual) {
            RouteInternalData route_internal_data;
            if (internal_data_usual) {
                route_internal_data.set_weight(internal_data_usual->weight);
            } else {
                route_internal_data.set_weight(-1);
                *(router_internal_data_list.add_route_internal_data()) = route_internal_data;
                continue;
            }
            if (internal_data_usual->prev_edge) {
                route_internal_data.set_prev_edge(*(internal_data_usual->prev_edge));
                route_internal_data.set_is_prev_edge(true);
            } else {
                route_internal_data.set_is_prev_edge(false);
            }
            *(router_internal_data_list.add_route_internal_data()) = route_internal_data;
        }
        *(result.add_route_internal_data_list()) = router_internal_data_list;
    }
    return result;
}


void Serializer::SerializeToOstream() const {
    ofstream out(requests_.GetRoot().AsMap().at("serialization_settings").AsMap().at("file").AsString());
    transport_catalogue_.SerializeToOstream(&out);
}
    

} // namespace serialization


