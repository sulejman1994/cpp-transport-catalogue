#include "json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    
    if (input.peek() == EOF) {
        throw ParsingError("parsing error");
    }
    Array result;
    const set<char> symbols = {' ', '\r', '\n', '\t'};
    
    for (char c; input >> noskipws >> c && c != ']';) {

        if (input.peek() == EOF) {
            throw ParsingError("parsing error");
        }
        if (symbols.count(c)) {
            continue;
        }
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    return Node(move(result));
}


Node LoadString(istream& input) {
    
    if (input.peek() == EOF) {
        throw ParsingError("parsing error");
    }
    if (input.peek() == '"') {
        input.ignore(1);
        return Node("");
    }
    
    string line;
    char prev, cur;
    prev = input.get();
    
    while (input.peek() != EOF) {
        cur = input.get();
        
        if (prev == '\\') {
            if (cur == '\\') {
                line += '\\';
            } else if (cur == 'n') {
                line += '\n';
            } else if (cur == 'r') {
                line += '\r';
            } else if (cur == 't') {
                line += '\t';
            } else if (cur == '"') {
                line += '"';
            } else {
                throw ParsingError("parsing error");
            }
            if (input.peek() == EOF) {
                throw ParsingError("parsing error");
            }
            prev = input.get();
            continue;
        } else {
            if (cur == '\n' || cur == '\r' || prev == '\n' || prev == '\r') {
                throw ParsingError("parsing error");
            }
            line += prev;
            if (cur == '"') {
                break;
            }
            if (input.peek() == EOF) {
                throw ParsingError("parsing error");
            }
        }
        prev = cur;
    }
    
    return Node(move(line));
}

Node LoadDict(istream& input) {
    if (input.peek() == EOF) {
        throw ParsingError("parsing error");
    }
    Dict result;
    const set<char> symbols = {' ', '\r', '\n', '\t'};

    for (char c; input >> noskipws >> c && c != '}';) {
        if (input.peek() == EOF) {
            throw ParsingError("parsing error");
        }
        if (symbols.count(c)) {
            continue;
        }
        if (c == ',') {
            input >> c;
        }
        
        while (c != '"') {
            if (input.peek() == EOF) {
                throw ParsingError("parsing error");
            }
            c = input.get();
        }
        
        string key = LoadString(input).AsString();
        
        while (c != ':') {
            if (input.peek() == EOF) {
                throw ParsingError("parsing error");
            }
            c = input.get();
        }
        
        result.insert({move(key), LoadNode(input)});
    }

    return Node(move(result));
}

void ParseDigits(istream& input, string& number_str) {
    char c = input.peek();
    while (!input.eof() && isdigit(input.peek())) {
        input >> c;
        number_str += c;
    }
}

void ParseWithFirstZero(istream& input, string& number_str) {
    char c;
    input >> c;
    if (c != '.' && c != 'e' && c != 'E') {
        input.putback(c);
        return;
    }
    number_str += c;
    
    if (c == 'e' || c == 'E') {
        if (input.peek() == '+') {
            input.ignore(1);
        } else if (input.peek() == '-') {
            input.ignore(1);
            number_str += '-';
        } else if (!isdigit(input.peek())) {
            throw ParsingError("parsing error");
        }
        ParseDigits(input, number_str);
        return;
    }
    
    if (!isdigit(input.peek())) {
        return;
    }
    ParseDigits(input, number_str);
    if (input.peek() != 'e' && input.peek() != 'E') {
        return;
    }
    input >> c;
    number_str += c;
    
    if (input.peek() == '+') {
        input.ignore(1);
    } else if (input.peek() == '-') {
        input.ignore(1);
        number_str += '-';
    } else if (!isdigit(input.peek())) {
        throw ParsingError("parsing error");
    }
    ParseDigits(input, number_str);
}

void ParseWithFirstNonZero(istream& input, string& number_str) {
    ParseDigits(input, number_str);
    ParseWithFirstZero(input, number_str);
}

Node LoadNumber(istream& input) {

    bool sign = true;
    if (input.peek() == '-') {
        sign = false;
        input.ignore(1);
    } else if (input.peek() == '+') {
        input.ignore(1);
    }
    
    char c;
    string number_str;

    input >> c;
    if (!isdigit(c)) {
        throw ParsingError("parsing error");
    }
    number_str += c;
    if (c == '0') {
        ParseWithFirstZero(input, number_str);
    } else {
        ParseWithFirstNonZero(input, number_str);
    }
    
    try {
        
    if (number_str.find('.') == string::npos && number_str.find('e') == string::npos && number_str.find('E') == string::npos) {
        if (!sign) {
            return Node( - stoi(number_str));
        }
        return Node(stoi(number_str));
    }
    
    if (!sign) {
        return Node( - stod(number_str));
    }
    return Node(stod(number_str));
        
    } catch (...) {
        throw ParsingError("parsing error");
    }
}

Node LoadBool(istream& input) {
    string word;
    for (int _ = 0; _ < 4; ++_) {
        if (!input || input.peek() == EOF) {
            throw ParsingError("parsing error");
        }
        word += input.peek();
        input.ignore(1);
    }
    if (word == "true") {
        return Node(true);
    }
    if (word == "fals" && input.peek() == 'e') {
        input.ignore(1);
        return Node(false);
    }
    throw ParsingError("parsing error");
}

Node LoadNull(istream& input) {
    string word;
    for (int _ = 0; _ < 4; ++_) {
        if (!input || input.peek() == EOF) {
            throw ParsingError("parsing error");
        }
        word += input.peek();
        input.ignore(1);
    }
    if (word == "null") {
        return Node(nullptr);
    }
    throw ParsingError("parsing error");
}

Node LoadNode(istream& input) {
 
    const set<char> symbols = {'{', '[', '"', '-', 't', 'f', 'n'};
    
    char c = input.get();
    
    while (true) {
        if (c == '"') {
            break;
        }
        if (c == '\\') {
            if (input.peek() == 'n' || input.peek() == 'r' || input.peek() == 't') {
                input.ignore(1);
                c = input.get();
                continue;
            }
            if (input.peek() == '"') {
                c = '"';
                input.ignore(1);
                break;
            }
        }
        if (c == ' ' || escape_symbols.count(c)) {
            c = input.get();
            continue;
        }
        if (!isdigit(c) && symbols.count(c) == 0) {
            throw ParsingError("parsing error");
        }
        break;
    }
    
    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
       // cout << "LoadString" << endl;
        return LoadString(input);
    } else if (c == 't' || c == 'f') {
        input.putback(c);
        return LoadBool(input);
    } else if (c == 'n') {
        input.putback(c);
        return LoadNull(input);
    } else {
        input.putback(c);
        return LoadNumber(input);
    }
}

}  // namespace




Node::Node()
    : variant_(nullptr) {
}

Node::Node(Array array)
    : variant_(move(array)) {
}

Node::Node(Dict map)
    : variant_(move(map)) {
}

Node::Node(double value)
    : variant_(value) {
}

Node::Node(int value)
    : variant_(value) {
}

Node::Node(string value)
    : variant_(move(value)) {
}

Node::Node(bool value)
    : variant_(value)  {
}

Node::Node(nullptr_t)
    : variant_(nullptr) {
}


bool Node::IsInt() const {
    return holds_alternative<int>(variant_);
}
bool Node::IsDouble() const {
    return holds_alternative<int>(variant_) || holds_alternative<double>(variant_);
}
bool Node::IsPureDouble() const {
    return !holds_alternative<int>(variant_) && holds_alternative<double>(variant_);
}
bool Node::IsBool() const {
    return holds_alternative<bool>(variant_);
}

bool Node::IsString() const {
    return holds_alternative<string>(variant_);
}
bool Node::IsNull() const {
    return holds_alternative<nullptr_t>(variant_);
}
bool Node::IsArray() const {
    return holds_alternative<Array>(variant_);
}
bool Node::IsMap() const {
    return holds_alternative<Dict>(variant_);
}

const Array& Node::AsArray() const {
    if (!holds_alternative<Array>(variant_)) {
        throw logic_error("invalid type");
    }
    return get<Array>(variant_);
}

const Dict& Node::AsMap() const {
    if (!holds_alternative<Dict>(variant_)) {
        throw logic_error("invalid type");
    }
    return get<Dict>(variant_);
}

int Node::AsInt() const {
    if (!holds_alternative<int>(variant_)) {
        throw logic_error("invalid type");
    }
    return get<int>(variant_);
}

double Node::AsDouble() const {
    if (!holds_alternative<int>(variant_) && !holds_alternative<double>(variant_)) {
        throw logic_error("invalid type");
    }
    if (holds_alternative<int>(variant_)) {
        return get<int>(variant_) * 1.0;
    }
    return get<double>(variant_);
}

bool Node::AsBool() const {
    if (!holds_alternative<bool>(variant_)) {
        throw logic_error("invalid type");
    }
    return get<bool>(variant_);
}

const string& Node::AsString() const {
    if (!holds_alternative<string>(variant_)) {
        throw logic_error("invalid type");
    }
    return get<string>(variant_);
}

bool Node::operator==(const Node& rhs) const {
    return variant_.index() == rhs.variant_.index() && variant_ == rhs.variant_;
}

bool Node::operator!=(const Node& rhs) const {
    return !(variant_ == rhs.variant_);
}

bool Document::operator == (const Document& rhs) const {
    return root_ == rhs.root_;
}

bool Document::operator != (const Document& rhs) const {
    return !(*this == rhs);
}

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

void PrintString(const string& str, ostream& output) {
    const set<char> special_chars = {'\n', '\r', '\"', '\\'};
    output << '"';
    for (char c : str) {
        if (c == '\n') {
            output << '\\' << 'n';
        } else if (c == '\r') {
            output << '\\' << 'r';
        } else if (c == '\t') {
            output << c;
        } else if (c == '\"') {
            output << '\\' << '"';
        } else if (c == '\\') {
            output << '\\' << '\\';
        } else {
            output << c;
        }
    }
   
    output << '"';
}

void PrintArray(const Node& root, ostream& output);

void PrintMap(const Node& root, ostream& output);


void Print(const Document& doc, ostream& output) {
    
    const Node& root = move(doc.GetRoot());

    if (root.IsNull()) {
        output << "null";
    } else if (root.IsBool()) {
        output << boolalpha << root.AsBool();
    } else if (root.IsString()) {
        PrintString(root.AsString(), output);
    } else if (root.IsInt()) {
        output << root.AsInt();
    } else if (root.IsDouble()) {
        output << root.AsDouble();
    } else if (root.IsArray()) {
        PrintArray(root, output);
    } else if (root.IsMap()) {
        PrintMap(root, output);
    }
}

void PrintArray(const Node& root, ostream& output) {
    output << "[";
    size_t counter = 0;
    size_t n = root.AsArray().size();
    for (const Node& node : root.AsArray()) {
        ++counter;
        Print(Document(node), output);
        if (counter != n) {
            output << ", ";
        }
    }
    output << "] ";
}

void PrintMap(const Node& root, ostream& output) {

    output << "{ ";
    size_t counter = 0;
    size_t n = root.AsMap().size();
    
    for (const auto& [key, node] : root.AsMap()) {
        ++counter;
        PrintString(key, output);
        output << " : ";
        Print(Document(node), output);
        if (counter != n) {
            output << ", ";
        }
    }
    output << " } ";
}

}  // namespace json

