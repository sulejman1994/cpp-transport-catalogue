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
    
    void PushEmptyArrayOrDict(const Node::Value& value);
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
    
    StartDictContext Value(const Node::Value& value);
    
    StartArrayContext StartArray();
    
    StartDictContext StartDict();
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
    
    StartArrayContext Value(const Node::Value& value);
    
    StartArrayContext StartArray();
    
    StartDictContext StartDict();
    
    Builder& EndArray();
};


} // namespace json


