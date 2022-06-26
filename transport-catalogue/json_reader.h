#pragma once

#include "transport_catalogue.h"
#include "json_builder.h"
#include "request_handler.h"
#include "router.h"

#include <iostream>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <sstream>

using transport_catalogue::TransportCatalogue, json::Node, request_handler::RequestHandler;

namespace json_reader {
    
    void ReadInputAndProcessRequests(std::istream& input, TransportCatalogue& transport_catalogue, std::ostream& output = std::cout);

    void ProcessBaseRequests(const std::vector<Node>& base_requests, TransportCatalogue& transport_catalogue);
    void ProcessStopWithoutDistances(const std::map<std::string, Node>& stop, TransportCatalogue& transport_catalogue);
    void ProcessDistances(const std::map<std::string, Node>& stop, TransportCatalogue& transport_catalogue);
    void ProcessBus(const std::map<std::string, Node>& bus, TransportCatalogue& transport_catalogue);

    void ReadRoutingSettings(const std::map<std::string, Node>& routing_settings, TransportCatalogue& transport_catalogue);
    
    void ReadRenderSettings(const std::map<std::string, Node>& settings, renderer::RenderSettings& render_settings);
    svg::Color ReadColor(const Node& underlayer_color_node);

    void ProcessStatRequests(const std::vector<Node>& stat_requests, const RequestHandler& request_handler, std::ostream& output = std::cout);
    std::pair<std::string, Node> ProcessStopStat(const std::string& stopname, const RequestHandler& request_handler);
    std::optional<std::vector<Node>> ProcessBusStat(const std::string& busname, const RequestHandler& request_handler);
    void ProcessRenderStat(std::map<std::string, Node>& result, const RequestHandler& request_handler);
    std::vector<json::Node> CalcRouteItems(const std::vector<graph::EdgeId>& edges, const graph::DirectedWeightedGraph<double>& graph, const std::vector<TransportRouter::Vertex>& id_to_vertex);

} // namespace json_reader

