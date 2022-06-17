#include "json_builder.h"

namespace json {

Value_after_key_Context KeyContext::Value(const Node::Value& value) {
    builder_.Value(value);
    return Value_after_key_Context(builder_);
}

StartArrayContext KeyContext::StartArray() {
    builder_.StartArray();
    return StartArrayContext(builder_);
}

StartDictContext KeyContext::StartDict() {
    builder_.StartDict();
    return StartDictContext(builder_);
}

KeyContext Value_after_key_Context::Key(const string& key) {
    builder_.Key(key);
    return KeyContext(builder_);
}

Builder& Value_after_key_Context::EndDict() {
    return builder_.EndDict();
}

KeyContext StartDictContext::Key(const string& key) {
    builder_.Key(key);
    return KeyContext(builder_);
}

Builder& StartDictContext::EndDict() {
    return builder_.EndDict();
}

Value_in_array_Context StartArrayContext::Value(const Node::Value& value) {
    builder_.Value(value);
    return Value_in_array_Context(builder_);
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

Value_in_array_Context Value_in_array_Context::Value(const Node::Value& value) {
    builder_.Value(value);
    return Value_in_array_Context(builder_);
}

Builder& Value_in_array_Context::EndArray() {
    return builder_.EndArray();
}

StartArrayContext Value_in_array_Context::StartArray() {
    builder_.StartArray();
    return StartArrayContext(builder_);
}

StartDictContext Value_in_array_Context::StartDict() {
    builder_.StartDict();
    return StartDictContext(builder_);
}

KeyContext Builder::Key(const string& key) {
    if (is_built_) {
        throw logic_error("");
    }
    
    if (is_prev_key) {
        throw logic_error("");
    }
    if (included_values_.empty() || !holds_alternative<Dict> (included_values_.top())) {
        throw logic_error("");
    }
    is_prev_key = true;
    keys_.push(key);
    return KeyContext(*this);
}

Builder& Builder::Value(const Node::Value& value) {
    if (is_built_) {
        throw logic_error("");
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
            throw logic_error("");
        }
        (get<Dict> (included_values_.top()))[keys_.top()] = Node(value);
        keys_.pop();
        is_prev_key = false;
        return *this;
    }
    throw logic_error("");
}

StartDictContext Builder::StartDict() {
    if (is_built_) {
        throw logic_error("");
    }
    if (!included_values_.empty() && !is_prev_key && !holds_alternative<Array>(included_values_.top()))  {
        throw logic_error("");
    }
    is_prev_key = false;
    included_values_.push(Dict());
    return StartDictContext(*this);
}

StartArrayContext Builder::StartArray() {
    if (is_built_) {
        throw logic_error("");
    }
    if (!included_values_.empty() && !is_prev_key && !holds_alternative<Array>(included_values_.top()))  {
        throw logic_error("");
    }
    is_prev_key = false;
    included_values_.push(Array());
    return StartArrayContext(*this);
}

Builder& Builder::EndDict() {
    if (is_built_) {
        throw logic_error("");
    }
    if (included_values_.empty() || !holds_alternative<Dict> (included_values_.top())) {
        throw logic_error("");
    }
    if (is_prev_key) {
        throw logic_error("");
    }
    Node::Value dict = included_values_.top();
    included_values_.pop();
    is_prev_key = false;
    return Value(dict);
}

Builder& Builder::EndArray() {
    if (is_built_) {
        throw logic_error("");
    }
    if (included_values_.empty() || !holds_alternative<Array> (included_values_.top())) {
        throw logic_error("");
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
    throw logic_error("");
}

} // namespace json

