#include "json_reader.h"
#include "serialization.h"

#include <optional>
#include <fstream>
#include <algorithm>
#include <cassert>

using namespace std;
using namespace geo;
using namespace json;
using namespace svg;
using namespace renderer;
using namespace graph;
using namespace domain;

namespace json_reader {

void ProcessRequests(istream& input, ostream& output) {
    
    const json::Document requests = Load(input);
    
    ifstream in(requests.GetRoot().AsMap().at("serialization_settings").AsMap().at("file").AsString());
    
    serialization::TransportCatalogue transport_catalogue_serialized;
    transport_catalogue_serialized.ParseFromIstream(&in);
    
    transport_catalogue::TransportCatalogue transport_catalogue(transport_catalogue_serialized);
    
    serialization::RenderSettings render_settings;
    renderer::MapRenderer map_renderer(transport_catalogue_serialized.render_settings());
    
    transport_catalogue::TransportRouter router(transport_catalogue_serialized, transport_catalogue);
           
    request_handler::RequestHandler request_handler(transport_catalogue, map_renderer, router);
    
    ProcessStatRequests(requests.GetRoot().AsMap().at("stat_requests").AsArray(), request_handler, output);
    
}


void ProcessStatRequests(const vector<Node>& stat_requests, const RequestHandler& request_handler, ostream& output) {
    
    json::Builder builder;
    json::StartArrayContext context = builder.StartArray();
    
    for (const Node& request_node : stat_requests) {
        int id = request_node.AsMap().at("id").AsInt();
        const string type = request_node.AsMap().at("type").AsString();
        json::StartDictContext context_second = context.StartDict().Key("request_id").Value(id);
        
        if (type == "Stop") {
            
            string stopname = request_node.AsMap().at("name").AsString();
            optional<vector<string>> buses_for_stop = request_handler.ProcessStopRequest(stopname);
            if (buses_for_stop) {
                vector<Node> bus_nodes(buses_for_stop.value().size());
                transform(buses_for_stop.value().begin(), buses_for_stop.value().end(), bus_nodes.begin(),
                    [] (const string& busname) {
                        return Node(busname);
                });
                context_second.Key("buses").Value(move(bus_nodes));
                
            } else {
                context_second.Key("error_message").Value("not found");
            }
            
        } else if (type == "Bus") {
            
            string name = request_node.AsMap().at("name").AsString();
            optional<request_handler::BusRequestResult> result = request_handler.ProcessBusRequest(name);
            if (result) {
                context_second.Key("route_length"s).Value((int) result->route_length);
                context_second.Key("stop_count"s).Value((int) result->stop_count);
                context_second.Key("curvature").Value(result->curvature);
                context_second.Key("unique_stop_count").Value((int) result->unique_stop_count);
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

} // namespace json_reader


