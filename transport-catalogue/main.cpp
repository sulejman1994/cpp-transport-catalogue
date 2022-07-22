#include "json_reader.h"
#include "serialization.h"

#include <iostream>
#include <cassert>
#include <fstream>
#include <ctime>
#include <string_view>
using namespace std;
using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);
    
    if (mode == "make_base"sv) {
        
        serialization::Serializer serializer;
        serializer.SerializeFromInput(cin);

    } else if (mode == "process_requests"sv) {

        json_reader::ProcessRequests(cin, cout);

    } else {
        PrintUsage();
        return 1;
    }
}
