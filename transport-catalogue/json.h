#pragma once

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <variant>

using std::map, std::set, std::string, std::vector, std::variant, std::ostream, std::istream, std::runtime_error, std::nullptr_t;

namespace json {

const set<char> escape_symbols = {'\n', '\t', '\r'};

class Node;

using Dict = map<string, Node>;
using Array = vector<Node>;

class ParsingError : public runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    
    using Value = variant<nullptr_t, int, double, string, bool, Array, Dict>;

    Node();
    Node(Array array);
    Node(Dict map);
    Node(double value);
    Node(int value);
    Node(string value);
    Node(bool value);
    Node(nullptr_t);
    Node(Value value);
        
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    const Array& AsArray() const;
    const Dict& AsMap() const;
    double AsDouble() const;
    int AsInt() const;
    bool AsBool() const;
    const string& AsString() const;
    
    const Value& GetValue() const;
    
    bool operator==(const Node& rhs) const;
    
    bool operator!=(const Node& rhs) const;
    
private:
    Value variant_;
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;
    
    bool operator == (const Document& rhs) const;
    
    bool operator != (const Document& rhs) const;

private:
    Node root_;
};

Document Load(istream& input);

void PrintString(const string& str, ostream& output);

void Print(const Document& doc, ostream& output);

}  // namespace json

