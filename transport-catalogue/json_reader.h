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
    
    void ProcessRequests(std::istream& input, std::ostream& output = std::cout);

    void ProcessStatRequests(const std::vector<Node>& stat_requests, const RequestHandler& request_handler, std::ostream& output = std::cout);

    std::vector<json::Node> CalcRouteItems(const std::vector<graph::EdgeId>& edges, const graph::DirectedWeightedGraph<double>& graph, const std::vector<TransportRouter::Vertex>& id_to_vertex);

} // namespace json_reader


