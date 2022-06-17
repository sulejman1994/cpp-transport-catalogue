#pragma once

#include "json.h"
#include <optional>
#include <stack>
using std::string, std::optional, std::stack, std::logic_error, std::holds_alternative, std::get;

namespace json {

class Context;
class KeyContext;
class StartDictContext;
class StartArrayContext;
class Value_after_key_Context;
class Value_in_array_Context;

class Builder {
public:
    
    KeyContext Key(const string& key);
    
    Builder& Value(const Node::Value& value);
    
    StartDictContext StartDict();
    
    StartArrayContext StartArray();
    
    Builder& EndDict();
    
    Builder& EndArray();
    
    Node Build();
    
private:
    Node::Value value_;
    stack<string> keys_;
    bool is_prev_key = false;
    bool is_built_ = false;
    stack<Node::Value> included_values_;
};

class Context {
public:
    Context(Builder& builder)
        : builder_(builder) {
    }
protected:
    Builder& builder_;
};

class KeyContext : public Context {
public:
    KeyContext(Builder& builder) : Context(builder) {
    }
    
    Value_after_key_Context Value(const Node::Value& value);
    
    StartArrayContext StartArray();
    
    StartDictContext StartDict();
};

class Value_after_key_Context : public Context {
public:
    Value_after_key_Context(Builder& builder) : Context(builder) {
    }
    
    KeyContext Key(const string& key);
    
    Builder& EndDict();
};

class StartDictContext : public Context {
public:
    StartDictContext(Builder& builder) : Context(builder) {
    }
    
    KeyContext Key(const string& key);
    
    Builder& EndDict();
};

class StartArrayContext : public Context {
public:
    StartArrayContext(Builder& builder) : Context(builder) {
    }
    
    Value_in_array_Context Value(const Node::Value& value);
    
    StartArrayContext StartArray();
    
    StartDictContext StartDict();
    
    Builder& EndArray();
};

class Value_in_array_Context : public Context {
public:                                         
    Value_in_array_Context(Builder& builder) : Context(builder) {
    }
    
    Value_in_array_Context Value(const Node::Value& value);
    
    Builder& EndArray();
    
    StartArrayContext StartArray();
    
    StartDictContext StartDict();
};

} // namespace json


