#include "input_reader.h"

#include <sstream>
#include <algorithm>
#include <unordered_set>
#include <queue>

using namespace global;
using std::getline, std::sort, std::unordered_set, std::istringstream, std::queue, std::endl, std::cout;

void input_reader::ReadInput(istream& input, TransportCatalogue& transport_catalogue) {
    
    vector<string> queries;
    CollectAndSortQueries(input, queries);
            
    for (const string& query : queries) {
        istringstream stream(query);
        string operation_code;
        stream >> operation_code;
        
        if (operation_code == "Stop") {
            ReadStopWithoutDistances(stream, transport_catalogue);
        }
        if (operation_code == "Bus") {
            ReadBus(stream, transport_catalogue);
        }
    }
    
    for (const string& query : queries) {
        istringstream stream(query);
        string operation_code;
        stream >> operation_code;
        
        if (operation_code == "Stop") {
            ReadStopDistances(stream, transport_catalogue);
        } else {
            break;
        }
    }
    
}

void input_reader::CollectAndSortQueries(istream& input, vector<string>& queries) {
    
    const int big_number = 1000000007;
    int count_of_queries;
    input >> count_of_queries;
    input.ignore(big_number, '\n');
    
    string query;
    vector<string> stop_queries, bus_queries;
    stop_queries.reserve(count_of_queries);
    bus_queries.reserve(count_of_queries);

    for (int i = 0; i < count_of_queries; ++i) {
        getline(input, query);
        size_t index = query.find_first_not_of(" ");
        if (query[index] == 'S') {
            stop_queries.push_back(query);
        }
        if (query[index] == 'B') {
            bus_queries.push_back(query);
        }
    }
    
    for (const auto& query : stop_queries) {
        queries.push_back(query);
    }
    for (const auto& query : bus_queries) {
        queries.push_back(query);
    }
}

void input_reader::ReadStopWithoutDistances(istringstream& stream, TransportCatalogue& transport_catalogue) {
    
    string stop;
    while (stream.peek() == ' ') {
        stream.ignore(1);
    }
    getline(stream, stop, ':');
    double lat, lng;
    string lat_str;
    getline(stream, lat_str, ',');
    lat = stod(lat_str);
    stream >> lng;
    transport_catalogue.AddStop(stop, {lat, lng});
}

void input_reader::ReadStopDistances(istringstream& stream, TransportCatalogue& transport_catalogue) {
    
    const int big_number = 1000000007;
    string stop;
    while (stream.peek() == ' ') {
        stream.ignore(1);
    }
    getline(stream, stop, ':');
    stream.ignore(big_number, ',');
    stream.ignore(big_number, ',');
    
    
    while (stream) {
        int distance;
        stream >> distance;
        stream.ignore(1, 'm');
        string skip_word_to;
        stream >> skip_word_to;
        while (stream.peek() == ' ') {
            stream.ignore(1);
        }
        string stop_to;
        getline(stream, stop_to, ',');
        while (stop_to.back() == ' ') {
            stop_to.pop_back();
        }
        if (!stream) {
            break;
        }
        transport_catalogue.SetDistanceBetweenStops(stop, stop_to, distance);
    }
}

void input_reader::ReadBus(istringstream& stream, TransportCatalogue& transport_catalogue) {
    
    string bus;
    while (stream.peek() == ' ') {
        stream.ignore(1);
    }
    getline(stream, bus, ':');
    
    string raw;
    getline(stream, raw);
    
    char separator;
    if (raw.find('>') != -1) {
        separator = '>';
    } else {
        separator = '-';
    }
    
    stream = istringstream(raw);
    
    bool is_cycle = true;
    if (separator == '-') {
        is_cycle = false;
    }
    
    vector<string> stops;
    
    while (stream) {
        while (stream.peek() == ' ') {
            stream.ignore(1);
        }
        if (!stream) {
            break;
        }
        string stop;
        getline(stream, stop, separator);
        while (stop.back() == ' ') {
            stop.pop_back();
        }
        stops.push_back(stop);
    }
    
    if (!is_cycle) {
        vector<string> reversed_path(stops);
        reversed_path.pop_back();
        reverse(reversed_path.begin(), reversed_path.end());
        for (const string& stop : reversed_path) {
            stops.push_back(stop);
        }
    }
    
    transport_catalogue.AddBus(bus, stops);
}
