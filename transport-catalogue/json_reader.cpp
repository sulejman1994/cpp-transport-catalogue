#include "json_reader.h"
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
using namespace std;
using namespace geo;
using namespace json;
using namespace svg;
using namespace renderer;

namespace json_reader {

void ReadInputAndProcessRequests(istream& input, TransportCatalogue& transport_catalogue, ostream& output) {
    
    const json::Document requests = Load(input);
    
    ProcessBaseRequests(requests.GetRoot().AsMap().at("base_requests").AsArray(), transport_catalogue);
    
    renderer::RenderSettings render_settings;
    ReadRenderSettings(requests.GetRoot().AsMap().at("render_settings").AsMap(), render_settings);
    renderer::MapRenderer map_renderer(render_settings);
    request_handler::RequestHandler request_handler(transport_catalogue, map_renderer);
    
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
  
    vector<Node> all_results;
    for (const Node& request_node : stat_requests) {
        int id = request_node.AsMap().at("id").AsInt();
        const string type = request_node.AsMap().at("type").AsString();
        map<string, Node> result;
        result["request_id"] = Node(id);
        
        if (type == "Stop") {
            string name = request_node.AsMap().at("name").AsString();
            ProcessStopStat(name, result, request_handler);
        } else if (type == "Bus") {
            string name = request_node.AsMap().at("name").AsString();
            ProcessBusStat(name, result, request_handler);
        } else if (type == "Map") {
            ProcessRenderStat(result, request_handler);
        }
        all_results.push_back(Node(result));
    }
    
    Print(json::Document(Node(all_results)), output);
}

void ProcessStopStat(const string& stopname, map<string, Node>& result, const RequestHandler& request_handler) {
   
    if (!request_handler.IsThereStop(stopname)) {
        result["error_message"] = Node("not found"s);
        return;
    }
    set<string> buses;
    for (const Bus* bus : request_handler.GetBusesForStop(stopname)) {
        buses.insert(bus->busname);
    }
    vector<Node> buses_nodes(buses.size());
    transform(buses.begin(), buses.end(), buses_nodes.begin(), [] (const string& bus) {
        return Node(bus);
    });
    result["buses"] = Node(buses_nodes);
}

void ProcessBusStat(const string& stopname, map<string, Node>& result, const RequestHandler& request_handler) {
    
    if (!request_handler.IsThereBus(stopname)) {
        result["error_message"] = Node("not found"s);
        return;
    }
    
    const Bus* bus = request_handler.GetBus(stopname);
    int stop_count = (int) bus->stops.size();
    
    int length = 0;
    double geo_length = 0;
        
    for (int i = 1; i < stop_count; ++i) {
        const Stop* prev_stop = bus->stops[i - 1];
        const Stop* cur_stop = bus->stops[i];
        length += request_handler.GetDistanceBetweenStops(prev_stop->stopname, cur_stop->stopname);
        geo_length += ComputeDistance(prev_stop->coordinates, cur_stop->coordinates);
        if (!bus->is_roundtrip) {
            length += request_handler.GetDistanceBetweenStops(cur_stop->stopname, prev_stop->stopname);
        }
    }
    
    if (!bus->is_roundtrip) {
        stop_count = stop_count * 2 - 1;
        geo_length *= 2;
    }
    
    result["route_length"] = Node(length);
    result["stop_count"] = Node(stop_count);
    
    double curvature = length * 1.0 / geo_length;
    result["curvature"] = Node(curvature * 1.0);
    
    unordered_set<const Stop*> unique_stops({bus->stops.begin(), bus->stops.end()});
    result["unique_stop_count"] = Node(int(unique_stops.size()));
}

void ProcessRenderStat(map<string, Node>& result, const RequestHandler& request_handler) {
    
    const svg::Document doc = request_handler.RenderMap();
    ostringstream stream;
    doc.Render(stream);
    result["map"] = Node(stream.str());
}


} // namespace json_reader
