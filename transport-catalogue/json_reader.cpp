#include "json_reader.h"

#include <optional>
#include <fstream>

using namespace std;
using namespace geo;
using namespace json;
using namespace svg;
using namespace renderer;
using namespace graph;
using namespace domain;

namespace json_reader {

void ReadInputAndProcessRequests(istream& input, TransportCatalogue& transport_catalogue, ostream& output) {
    
    const json::Document requests = Load(input);
        
    ProcessBaseRequests(requests.GetRoot().AsMap().at("base_requests").AsArray(), transport_catalogue);
    
    renderer::RenderSettings render_settings;
    ReadRenderSettings(requests.GetRoot().AsMap().at("render_settings").AsMap(), render_settings);
    renderer::MapRenderer map_renderer(render_settings);
    
    ReadRoutingSettings(requests.GetRoot().AsMap().at("routing_settings").AsMap(), transport_catalogue);
    transport_catalogue::TransportRouter router(transport_catalogue);
    request_handler::RequestHandler request_handler(transport_catalogue, map_renderer, router);
    
    ProcessStatRequests(requests.GetRoot().AsMap().at("stat_requests").AsArray(), request_handler, output);
    
}

void ProcessBaseRequests(const vector<Node>& base_requests, TransportCatalogue& transport_catalogue) {

    for (const Node& node : base_requests) {
        if (node.AsMap().at("type").AsString() == "Stop") {
            ProcessStopWithoutDistances(node.AsMap(), transport_catalogue);
        }
    }
    
    for (const Node& node : base_requests) {
        if (node.AsMap().at("type").AsString() == "Stop") {
            ProcessDistances(node.AsMap(), transport_catalogue);
        }
        if (node.AsMap().at("type").AsString() == "Bus") {
            ProcessBus(node.AsMap(), transport_catalogue);
        }
    }
}

void ProcessStopWithoutDistances(const map<string, Node>& stop, TransportCatalogue& transport_catalogue) {
    
    transport_catalogue.AddStop(stop.at("name").AsString(), {stop.at("latitude").AsDouble(), stop.at("longitude").AsDouble()});
}

void ProcessDistances(const map<string, Node>& stop, TransportCatalogue& transport_catalogue) {
  
    string stop_from = stop.at("name").AsString();
    for (const auto& [stop_to, distance] : stop.at("road_distances").AsMap()) {
        transport_catalogue.SetDistanceBetweenStops(stop_from, stop_to, distance.AsInt());
    }
}


void ProcessBus(const map<string, Node>& bus, TransportCatalogue& transport_catalogue) {
   
    string busname = bus.at("name").AsString();
    bool is_roundtrip = bus.at("is_roundtrip").AsBool();
    vector<string> stops;
    for (const Node& stopname_node : bus.at("stops").AsArray()) {
        stops.push_back(stopname_node.AsString());
    }
    transport_catalogue.AddBus(busname, stops, is_roundtrip);
}

void ReadRoutingSettings(const map<string, Node>& routing_settings, TransportCatalogue& transport_catalogue) {
    transport_catalogue.SetBusWaitTime(routing_settings.at("bus_wait_time").AsInt());
    transport_catalogue.SetBusVelocity(routing_settings.at("bus_velocity").AsInt());
}


void ReadRenderSettings(const map<string, Node>& settings, renderer::RenderSettings& render_settings) {
    render_settings.width = settings.at("width").AsDouble();
    render_settings.height = settings.at("height").AsDouble();
    render_settings.padding = settings.at("padding").AsDouble();
    render_settings.line_width = settings.at("line_width").AsDouble();
    render_settings.stop_radius = settings.at("stop_radius").AsDouble();
    render_settings.bus_label_font_size = settings.at("bus_label_font_size").AsInt();
    render_settings.stop_label_font_size = settings.at("stop_label_font_size").AsInt();
    render_settings.underlayer_width = settings.at("underlayer_width").AsDouble();
    
    render_settings.bus_label_offset = { settings.at("bus_label_offset").AsArray()[0].AsDouble(), settings.at("bus_label_offset").AsArray()[1].AsDouble() };
    
    render_settings.stop_label_offset = { settings.at("stop_label_offset").AsArray()[0].AsDouble(), settings.at("stop_label_offset").AsArray()[1].AsDouble() };
    
    render_settings.underlayer_color = ReadColor(settings.at("underlayer_color"));
    
    for (const Node& color_node : settings.at("color_palette").AsArray()) {
        render_settings.color_palette.push_back(ReadColor(color_node));
    }
}

Color ReadColor(const Node& color_node) {

    int red, green, blue;
    if (color_node.IsString()) {
        return Color(color_node.AsString());
    }
    red = color_node.AsArray()[0].AsInt();
    green = color_node.AsArray()[1].AsInt();
    blue = color_node.AsArray()[2].AsInt();
    if (color_node.AsArray().size() == 3) {
        return Rgb(red, green, blue);
    }
    double opasity = color_node.AsArray()[3].AsDouble();
    return Rgba(red, green, blue, opasity);
}

void ProcessStatRequests(const vector<Node>& stat_requests, const RequestHandler& request_handler, ostream& output) {
    
    Builder builder;
    StartArrayContext context = builder.StartArray();
    
    
    for (const Node& request_node : stat_requests) {
        int id = request_node.AsMap().at("id").AsInt();
        const string type = request_node.AsMap().at("type").AsString();
        StartDictContext context_second = context.StartDict().Key("request_id").Value(id);
        
        if (type == "Stop") {
            
            string stopname = request_node.AsMap().at("name").AsString();
            pair<string, Node> result(ProcessStopStat(stopname, request_handler));
            context_second.Key(result.first).Value(result.second.GetValue());
            
        } else if (type == "Bus") {
            
            string name = request_node.AsMap().at("name").AsString();
            optional<vector<Node>> result(ProcessBusStat(name, request_handler));
            if (result) {
                context_second.Key("route_length"s).Value((*(result))[0].GetValue());
                context_second.Key("stop_count"s).Value((*(result))[1].GetValue());
                context_second.Key("curvature").Value((*(result))[2].GetValue());
                context_second.Key("unique_stop_count").Value((*(result))[3].GetValue());
            } else {
                context_second.Key("error_message"s).Value("not found"s);
            }
            
        } else if (type == "Map") {
            
            const svg::Document doc = request_handler.RenderMap();
            ostringstream stream;
            doc.Render(stream);
            context_second.Key("map").Value(stream.str());
            
        } else if (type == "Route") {
            
            string from = request_node.AsMap().at("from").AsString();
            string to = request_node.AsMap().at("to").AsString();
            
            auto route = request_handler.BuildRoute(from, to);
            if (route) {
                context_second.Key("total_time").Value(route->weight);
                vector<Node> items = CalcRouteItems(route->edges, request_handler.GetRouter().GetGraph(), request_handler.GetRouter().GetIdToVertex());
                context_second.Key("items").Value(items);
            } else {
                context_second.Key("error_message").Value("not found");
            }
            
        }
        context_second.EndDict();
    }
    
    context.EndArray();

    Print(json::Document(builder.Build()), output);
}

vector<Node> CalcRouteItems(const vector<EdgeId>& edges, const DirectedWeightedGraph<double>& graph, const vector<TransportRouter::Vertex>& id_to_vertex) {
    
    vector<Node> result;
    result.reserve(edges.size());
    for (EdgeId edge_id : edges) {
        Edge edge = graph.GetEdge(edge_id);
        map<string, Node> res;
        res["time"s] = Node(edge.weight);
        if (edge.stop_count == 0) {
            res["type"s] = Node("Wait"s);
            res["stop_name"s] = Node(id_to_vertex[edge.from].stopname);
        } else {
            res["type"s] = Node("Bus"s);
            res["span_count"s] = Node((int) edge.stop_count);
            res["bus"s] = Node(edge.busname);
        }
        result.push_back(res);
    }
    return result;
}


pair<string, Node> ProcessStopStat(const string& stopname, const RequestHandler& request_handler) {
    
    if (!request_handler.IsThereStop(stopname)) {
        return {"error_message"s, Node("not found"s)};
    }
    set<string> buses;
    for (const Bus* bus : request_handler.GetBusesForStop(stopname)) {
        buses.insert(bus->name);
    }
    vector<Node> buses_nodes(buses.size());
    transform(buses.begin(), buses.end(), buses_nodes.begin(), [] (const string& bus) {
        return Node(bus);
    });
    return {"buses"s, Node(move(buses_nodes))};
}

optional<vector<Node>> ProcessBusStat(const string& busname, const RequestHandler& request_handler) {
    
    if (!request_handler.IsThereBus(busname)) {
        return {};
    }
    const Bus* bus = request_handler.GetBus(busname);
    int stop_count = (int) bus->stops.size();
    
    int length = 0;
    double geo_length = 0;
        
    for (int i = 1; i < stop_count; ++i) {
        const Stop* prev_stop = bus->stops[i - 1];
        const Stop* cur_stop = bus->stops[i];
        length += request_handler.GetDistanceBetweenStops(prev_stop->name, cur_stop->name);
        geo_length += ComputeDistance(prev_stop->coordinates, cur_stop->coordinates);
        if (!bus->is_roundtrip) {
            length += request_handler.GetDistanceBetweenStops(cur_stop->name, prev_stop->name);
        }
    }
    
    if (!bus->is_roundtrip) {
        stop_count = stop_count * 2 - 1;
        geo_length *= 2;
    }
    
    double curvature = length * 1.0 / geo_length;
    unordered_set<const Stop*> unique_stops({bus->stops.begin(), bus->stops.end()});
    int unique_stops_count = (int) unique_stops.size();
    vector<Node> result = {length, stop_count, curvature, unique_stops_count};
    return optional(result);
}

} // namespace json_reader
