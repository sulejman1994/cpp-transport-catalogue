#pragma once

#include "transport_catalogue.h"
#include "json_builder.h"
#include "request_handler.h"

#include <iostream>
#include <map>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <sstream>

using transport_catalogue::TransportCatalogue, json::Node, request_handler::RequestHandler, std::istream, std::ostream, std::map, std::unordered_set, std::cout, std::set, std::transform;

namespace json_reader {
    
    void ReadInputAndProcessRequests(istream& input, TransportCatalogue& transport_catalogue, ostream& output = cout);

    void ProcessBaseRequests(const vector<Node>& base_requests, TransportCatalogue& transport_catalogue);

    void ProcessStopWithoutDistances(const map<string, Node>& stop, TransportCatalogue& transport_catalogue);

    void ProcessDistances(const map<string, Node>& stop, TransportCatalogue& transport_catalogue);

    void ProcessBus(const map<string, Node>& bus, TransportCatalogue& transport_catalogue);

    void ReadRenderSettings(const map<string, Node>& settings, renderer::RenderSettings& render_settings);

    svg::Color ReadColor(const Node& underlayer_color_node);

    void ProcessRenderStat(map<string, Node>& result, const RequestHandler& request_handler);

    void ProcessStatRequests(const vector<Node>& stat_requests, const RequestHandler& request_handler, ostream& output = cout);
    
    pair<string, Node> ProcessStopStat(const string& stopname, const RequestHandler& request_handler);

    optional<vector<Node>> ProcessBusStat(const string& busname, const RequestHandler& request_handler);


} // namespace json_reader

