#pragma once

#include <variant>
#include <string>
#include <memory>
#include <unordered_map>
#include <fstream>
#include <defines.h>
#include <iostream>
class JsonArray;
class JsonObject;

using ValueType = std::variant<std::string, std::shared_ptr<JsonObject>,  std::shared_ptr<JsonArray>, i32, i64, f32, f64, bool, std::nullptr_t>;

static int count = 0;

enum JsonType {
    JT_STRING,
    JT_OBJECT,
    JT_ARRAY,
    JT_INT,
    JT_INT_64,
    JT_FLOAT,
    JT_DOUBLE,
    JT_BOOL,
    JT_NULL,
    JT_NONE,
};

enum JsonParentType {
    JPT_NONE,
    JPT_OBJECT,
    JPT_ARRAY,
};

struct JsonValueType {
    ValueType value;
    JsonType type;
};


struct JsonArray {
    std::vector<std::shared_ptr<JsonValueType>> array;
    std::variant<std::shared_ptr<JsonObject>, std::shared_ptr<JsonArray>> parent;
    JsonParentType parent_type;

    i32 size() {
        return array.size();
    }

    template<typename T>
    T get(i32 index) {
        auto value = array.at(index);
        return std::get<T>(value->value);
    }

    void reset_parent() {
        if (parent_type == JPT_OBJECT) {
            auto ptr = std::get<std::shared_ptr<JsonObject>>(parent);
            ptr.reset();
        }
        if (parent_type == JPT_ARRAY) {
            auto ptr = std::get<std::shared_ptr<JsonArray>>(parent);
            ptr.reset();
        }
    }
};




struct JsonObject {
    std::vector<std::string> keys;
    std::unordered_map<std::string,  std::shared_ptr<JsonValueType>> key_value_pair;
    std::variant<std::shared_ptr<JsonObject>, std::shared_ptr<JsonArray>> parent;
    JsonParentType parent_type {JPT_NONE};

    template<typename T>
    T get(const std::string key) {
        std::shared_ptr<JsonValueType> object = key_value_pair.at(key);
        return  std::get<T>(object->value);
    }

    JsonType get_type(const std::string key) {
        std::shared_ptr<JsonValueType> object = key_value_pair.at(key);
        return object->type;
    }

    void reset_parent() {
        if (parent_type == JPT_OBJECT) {
            auto ptr = std::get<std::shared_ptr<JsonObject>>(parent);
            ptr.reset();
        }
        if (parent_type == JPT_ARRAY) {
            auto ptr = std::get<std::shared_ptr<JsonArray>>(parent);
            ptr.reset();
        }
    }
};

using JsonObjectSPtr = std::shared_ptr<JsonObject>;
using  JsonValueTypeSPtr = std::shared_ptr<JsonValueType>;
using JsonArraySPtr = std::shared_ptr<JsonArray>;


class Json {
public:
    Json(const std::string& file_path);
    ~Json() = default;
    JsonObjectSPtr parse();
    void reset_ptr(std::variant<std::shared_ptr<JsonObject>, std::shared_ptr<JsonArray>>& parent, JsonParentType type);
    void reset_parent(JsonObjectSPtr& root);
    void reset_parent(JsonArraySPtr& array);
private:
    i32 next_state;
    i32 previous_state;

    JsonObjectSPtr root;
    JsonObjectSPtr current_object;

    JsonArraySPtr current_array;
    bool b_is_in_array {false};

    char current_character;
    std::ifstream json_file;
    bool eof_reached {false};
    std::string current_key;
    u64 current_line_number {1};
    bool has_minus {false};
    std::unordered_map<std::string, u64> checker;
    // functions
    bool find_character(char character);
    std::pair<bool, std::string> get_key();
    void get_value();
    void find_next_value_or_closure();

    void error(const std::string& msg);

    std::pair<JsonType, f64> convert_string_to_float(const std::string& float_str);
    std::pair<JsonType, i64> convert_string_to_int(const std::string& float_str);

    

};