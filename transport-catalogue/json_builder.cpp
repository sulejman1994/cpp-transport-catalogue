#include "json_builder.h"

namespace json {

StartDictContext KeyContext::Value(const Node::Value& value) {
    builder_.Value(value);
    return StartDictContext(builder_);
}

StartArrayContext KeyContext::StartArray() {
    builder_.StartArray();
    return StartArrayContext(builder_);
}

StartDictContext KeyContext::StartDict() {
    builder_.StartDict();
    return StartDictContext(builder_);
}

KeyContext StartDictContext::Key(const string& key) {
    builder_.Key(key);
    return KeyContext(builder_);
}

Builder& StartDictContext::EndDict() {
    return builder_.EndDict();
}

StartArrayContext StartArrayContext::Value(const Node::Value& value) {
    builder_.Value(value);
    return StartArrayContext(builder_);
}

StartArrayContext StartArrayContext::StartArray() {
    builder_.StartArray();
    return StartArrayContext(builder_);
}

StartDictContext StartArrayContext::StartDict() {
    builder_.StartDict();
    return StartDictContext(builder_);
}

Builder& StartArrayContext::EndArray() {
    return builder_.EndArray();
}


KeyContext Builder::Key(const string& key) {
    if (is_built_) {
        throw logic_error("expected building");
    }
    
    if (is_prev_key) {
        throw logic_error("expected value");
    }
    if (included_values_.empty() || !holds_alternative<Dict> (included_values_.top())) {
        throw logic_error("not a dict");
    }
    is_prev_key = true;
    keys_.push(key);
    return KeyContext(*this);
}

Builder& Builder::Value(const Node::Value& value) {
    if (is_built_) {
        throw logic_error("expected building");
    }
    
    is_prev_key = false;
    if (included_values_.empty()) {
        value_ = value;
        is_built_ = true;
        return *this;
    }
    if (holds_alternative<Array> (included_values_.top())) {
        (get<Array> (included_values_.top())).push_back(Node(value));
        return *this;
    }
    if (holds_alternative<Dict> (included_values_.top())) {
        if (keys_.empty()) {
            throw logic_error("expected key");
        }
        (get<Dict> (included_values_.top()))[keys_.top()] = Node(value);
        keys_.pop();
        is_prev_key = false;
        return *this;
    }
    throw logic_error("invalid command");
}

StartDictContext Builder::StartDict() {
    PushEmptyArrayOrDict(Dict());
    return StartDictContext(*this);
}

StartArrayContext Builder::StartArray() {
    PushEmptyArrayOrDict(Array());
    return StartArrayContext(*this);
}

Builder& Builder::EndDict() {
    if (is_built_) {
        throw logic_error("expected building");
    }
    if (included_values_.empty() || !holds_alternative<Dict> (included_values_.top())) {
        throw logic_error("not dict");
    }
    if (is_prev_key) {
        throw logic_error("expected value");
    }
    Node::Value dict = included_values_.top();
    included_values_.pop();
    is_prev_key = false;
    return Value(dict);
}

Builder& Builder::EndArray() {
    if (is_built_) {
        throw logic_error("expected building");
    }
    if (included_values_.empty() || !holds_alternative<Array> (included_values_.top())) {
        throw logic_error("not array");
    }
    Node::Value array = included_values_.top();
    included_values_.pop();
    is_prev_key = false;
    return Value(array);
}

Node Builder::Build() {
    if (is_built_) {
        return Node(value_);
    }
    throw logic_error("has not built yet");
}

void Builder::PushEmptyArrayOrDict(const Node::Value& value) {
    if (is_built_) {
        throw logic_error("expected building");
    }
    if (!included_values_.empty() && !is_prev_key && !holds_alternative<Array>(included_values_.top()))  {
        throw logic_error("invalid command");
    }
    is_prev_key = false;
    included_values_.push(value);
}

} // namespace json

