#pragma once

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <variant>

namespace json {

const std::set<char> escape_symbols = {'\n', '\t', '\r'};

class Node;

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    
    using Value = std::variant<nullptr_t, int, double, std::string, bool, Array, Dict>;

    Node();
    Node(Array array);
    Node(Dict map);
    Node(double value);
    Node(int value);
    Node(std::string value);
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
    const std::string& AsString() const;
    
    const Value& GetValue() const;
    
    bool operator==(const Node& rhs) const;
    
    bool operator!=(const Node& rhs) const;
    
private:
    Value variant_;
};

class Document {
public:
    
    Document() = default;
    explicit Document(Node root);

    const Node& GetRoot() const;
    
    bool operator == (const Document& rhs) const;
    
    bool operator != (const Document& rhs) const;

private:
    Node root_;
};

Document Load(std::istream& input);

void PrintString(const std::string& str, std::ostream& output);

void Print(const Document& doc, std::ostream& output);

}  // namespace json


