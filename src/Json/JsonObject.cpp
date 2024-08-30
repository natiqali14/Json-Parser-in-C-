#include "JsonObject.h"
#include <sstream>
#include <iostream>
#include <string>

/* ------------------------------------  Some helper macros  ------------------------------------ */

#define special_skip_chars(j_char)  (j_char == ' ' || j_char == '\n' || j_char == '\r' || j_char == '\t')
#define SKIP_CHARACTERS(j_char,n) {   \
    if (j_char == '\n') n++; \
    if special_skip_chars(j_char) {continue;} \
}

#define make_shared_pointer(class) std::make_shared<class>();

/* ------------------------------------ Internal enum to parsing states  ------------------------------------ */

enum pointer_state {
    OPEN_BRACKET,
    FIND_DOUBLE_QUOTE,
    CLOSE_BRACKET,
    COLON,
    GET_KEY,
    GET_VALUE,
    AFTER_VALUE_CHECK,
    FIND_NEXT_CHARACTER,
    FIND_NEXT_VALUE_OR_CLOSURE,
    NONE,

};

/* ------------------------------------  Class member funcitons  ------------------------------------ */

Json::Json(const std::string& file_path)
{
    json_file.open(file_path);
    if (!json_file.is_open()) throw std::runtime_error("Can't open file");
    bool success = find_character('{');
    if (!success) throw std::runtime_error("Can't find first { bracket");
    checker.insert({"{", 1});
    root = make_shared_pointer(JsonObject);
    current_object = root;
    current_object->parent_type = JPT_OBJECT;
    next_state = GET_KEY;

}


std::pair<JsonType, f64> Json::convert_string_to_float(const std::string& float_str) {
    try {
        f32 f = std::stof(float_str);
        return {JT_FLOAT, f};
    }
    catch (std::invalid_argument e) {
        return {JT_NONE, 0.0f};
    }
    catch (std::out_of_range e) {
        try {
            f64 d = std::stod(float_str);
            return {JT_DOUBLE, d};
        }
        catch (...) {
            error("Unable to convert string to a floating point");
        }
    }
    
}

std::pair<JsonType, i64> Json::convert_string_to_int(const std::string& int_str) {
    try {
        i32 i = std::stoi(int_str);
        return {JT_INT, i};
    }
    catch (std::invalid_argument e) {
        return {JT_NONE, 0};
    }
    catch (std::out_of_range e) {
        try {
            i64 i = std::stoll(int_str);
            return {JT_INT_64, i};
        }
        catch (...) {
            error("Unable to convert string to a int type");
        }
    }
}

void Json::reset_ptr(std::variant<std::shared_ptr<JsonObject>, std::shared_ptr<JsonArray>>&parent, JsonParentType parent_type)
{
    if (parent_type == JPT_OBJECT) {
        std::get<std::shared_ptr<JsonObject>>(parent).reset();
    }
    if (parent_type == JPT_ARRAY) {
        std::get<std::shared_ptr<JsonArray>>(parent).reset();
    }
}

void Json::reset_parent(JsonObjectSPtr& root)
{
    for(auto& pair : root->key_value_pair) {
        auto& value = pair.second;
        if (value->type == JT_OBJECT) {
            auto& obj = std::get<JsonObjectSPtr>(value->value);
            reset_parent(obj);
            reset_ptr(obj->parent, obj->parent_type);
        }

        if (value->type == JT_ARRAY) {
            auto& obj = std::get<JsonArraySPtr>(value->value);
            reset_parent(obj);
            reset_ptr(obj->parent, obj->parent_type);
        }
    } 
}

void Json::reset_parent(JsonArraySPtr &array)
{
    for(auto& index : array->array) {
        auto& value = index;
        if (value->type == JT_OBJECT) {
            auto& obj = std::get<JsonObjectSPtr>(value->value);
            reset_parent(obj);
            reset_ptr(obj->parent, obj->parent_type);
        }

        if (value->type == JT_ARRAY) {
            auto& obj = std::get<JsonArraySPtr>(value->value);
            reset_parent(obj);
            reset_ptr(obj->parent, obj->parent_type);
        }
    }
}

JsonObjectSPtr Json::parse()
{
    
    while (!eof_reached) {
       // json_file.get(current_character);
        if (json_file.eof()) {
            eof_reached = true; continue;
        }

        switch (next_state)
        {
        case GET_KEY: {
            auto ret = get_key();
            if (!ret.first && ret.second.length() == 0) break;
            current_object->keys.push_back(ret.second);
            current_key = ret.second;
            previous_state = next_state;
            next_state = COLON;
        }
            break;
        case COLON: {
            auto ret = find_character(':');
            previous_state = next_state;
            next_state = GET_VALUE;
        }
        break;
        case GET_VALUE: {
            get_value();
            break;
        }
        case FIND_NEXT_VALUE_OR_CLOSURE: {
            find_next_value_or_closure();
            break;
        }
        default:
            break;
        }
    }
    if (checker["["] > 0 || checker["{"] > 0) error("Someting went wrong number of { and  } or [ and ] are not equal");
    return root;
}



bool Json::find_character(char character)
{
    while (json_file.get(current_character)) {
        SKIP_CHARACTERS(current_character, current_line_number);
        if (character != current_character) {
            std::stringstream ss;
            ss << "Can not find character ";
            ss << character;
            error(ss.str());
        }
        if(character == current_character) return true;
    }
    std::stringstream ss;
    ss << "Can not find character ";
    ss << character;
    if (json_file.eof()) throw std::runtime_error(ss.str().c_str());
    return false;   
}

std::pair<bool, std::string> Json::get_key()
{
    while (json_file.get(current_character))
    {
        SKIP_CHARACTERS(current_character, current_line_number);
        break;
    }
    if(current_character == '}') {
        if (checker["{"] <= 0) error("Extra } in file");
        previous_state = next_state;
        next_state = FIND_NEXT_VALUE_OR_CLOSURE;
        json_file.unget();
        checker["{"]++;
        return {false, ""};
    }
    if (current_character != '\"') error("not valild key starting");
    std::stringstream key;
    bool key_possed = false;
    while (json_file.get(current_character)) {
        if (current_character == '\n') error("Unexpected end of stream");
        if(current_character == '\"') break;
        if (json_file.eof()) error("not valid key ending");
        key << current_character;
    }
    if (key.str().length() == 0) error("provided empty key");
    return {true, key.str()};


}

void Json::get_value()
{
    while (json_file.get(current_character))
    {
        SKIP_CHARACTERS(current_character, current_line_number);
        break;
    }
    switch (current_character)
    {
        case '{': 
        {
            if (!b_is_in_array) 
            {
                // not in array
                // json object
                JsonObjectSPtr new_object = make_shared_pointer(JsonObject);
                new_object->parent = current_object;
                new_object->parent_type = JPT_OBJECT;

                // value object
                JsonValueTypeSPtr value = make_shared_pointer(JsonValueType);
                value->type = JT_OBJECT;
                value->value = new_object;
                if (current_object->key_value_pair.count(current_key)) error("Found duplicate key here");
                current_object->key_value_pair.insert({current_key, value});

                current_object = new_object;
            } 
            else 
            {
                // in array
                // json object
                JsonObjectSPtr new_object = make_shared_pointer(JsonObject);
                new_object->parent = current_array;
                new_object->parent_type = JPT_ARRAY;

                // value object
                JsonValueTypeSPtr value = make_shared_pointer(JsonValueType);
                value->type = JT_OBJECT;
                value->value = new_object;
                current_array->array.push_back(value);

                current_object = new_object;
                b_is_in_array = false;

            }

            previous_state = next_state;
            next_state = GET_KEY;
            checker["{"]++;
        }
            break;

        case '[': 
        {
            if (!b_is_in_array) 
            {
                JsonArraySPtr new_array = make_shared_pointer(JsonArray);
                new_array->parent = current_object;
                new_array->parent_type = JPT_OBJECT;

                JsonValueTypeSPtr value = make_shared_pointer(JsonValueType);
                value->type = JT_ARRAY;
                value->value = new_array;
                if (current_object->key_value_pair.count(current_key)) error("Found duplicate key here");
                current_object->key_value_pair.insert({current_key, value});

                current_array = new_array;
                b_is_in_array = true;
            }

            else 
            {
                JsonArraySPtr new_array = make_shared_pointer(JsonArray);
                new_array->parent = current_array;
                new_array->parent_type = JPT_ARRAY;

                JsonValueTypeSPtr value = make_shared_pointer(JsonValueType);
                value->type = JT_ARRAY;
                value->value = new_array;
                current_array->array.push_back(value);

                current_array = new_array;
            }

            previous_state = next_state;
            next_state = GET_VALUE;
            checker["["]++;
            break;
        }

        case ']':
        {
            if (checker["["] <= 0) error("Extra ] in file");
            previous_state = next_state;
            next_state = FIND_NEXT_VALUE_OR_CLOSURE;
            json_file.unget();
            checker["["]--;
            break;
        }

        case '\"': 
        {
            std::stringstream ss;
            JsonValueTypeSPtr value = make_shared_pointer(JsonValueType);
            value->type = JT_STRING;
            while (json_file.get(current_character) && current_character != '\"') 
            {
                if (current_character == '\n') error("Unexpected end of stream");
                ss << current_character;
            }
            value->value = ss.str();
            if (!b_is_in_array) 
            {
                if (current_object->key_value_pair.count(current_key)) error("Found duplicate key here");
                current_object->key_value_pair.insert({current_key, value});
            }
            else 
            {
                current_array->array.push_back(value);
            }
            
            
            previous_state = next_state;
            next_state = FIND_NEXT_VALUE_OR_CLOSURE;
            break;
        }
        case 45: // for '-'
        {
            has_minus = true;
            json_file.get(current_character);
            if (current_character >= 48 && current_character <= 57) {
                json_file.unget();
                previous_state = next_state;
                next_state = GET_VALUE;
                break;
            }
            else {
                error("Found something else other a digit after - sign");
            }
        }
        case 48 ... 57:  // for 1 to 9
        {
            std::stringstream ss; 
            if(has_minus) {
                ss << '-';
                has_minus = false;
            }
            ss << current_character;
            JsonValueTypeSPtr value = make_shared_pointer(JsonValueType);
            value->type = JT_STRING;
            bool decimal_point_found = false;
            if (current_character == '-') ss << current_character;
            while (json_file.get(current_character) && ((current_character >= 48 && current_character <= 57) || current_character == 46)) 
            {
                if (current_character == 46) decimal_point_found = true;
                ss << current_character;
            }

            if (decimal_point_found) {
                auto new_ret = convert_string_to_float(ss.str());
                if (new_ret.first == JT_NONE) error("Illegal digits");
                value->type = new_ret.first;
                if (value->type == JT_FLOAT) value->value = static_cast<f32>(new_ret.second);
                else value->value = new_ret.second;
            }
            else {
                auto ret = convert_string_to_int(ss.str());

                if (ret.first != JT_NONE) {
                    value->type = ret.first;
                    if (value->type == JT_INT) value->value = static_cast<i32>(ret.second);
                    else value->value = ret.second;
                }
                else error("Illegal digits");
            }

            if (!b_is_in_array) 
            {
                if (current_object->key_value_pair.count(current_key)) error("Found duplicate key here");
                current_object->key_value_pair.insert({current_key, value});
            }
            else 
            {
                current_array->array.push_back(value);
            }

            previous_state = next_state;
            next_state = FIND_NEXT_VALUE_OR_CLOSURE;
            json_file.unget();
            break;
        }
        case 't':
        {
            json_file.get(current_character);
            if (current_character != 'r') error("Wrong entry, may be going for true as  char was t");
            json_file.get(current_character);
            if (current_character != 'u') error("Wrong entry, may be going for true as  char was r");
            json_file.get(current_character);
            if (current_character != 'e') error("Wrong entry, may be going for true as  char was u");

            JsonValueTypeSPtr value = make_shared_pointer(JsonValueType);
            value->type = JT_BOOL;
            value->value = true;

            if (!b_is_in_array) 
            {
                if (current_object->key_value_pair.count(current_key)) error("Found duplicate key here");
                current_object->key_value_pair.insert({current_key, value});
            }
            else 
            {
                current_array->array.push_back(value);
            }

            previous_state = next_state;
            next_state = FIND_NEXT_VALUE_OR_CLOSURE;
            break;
        }

        case 'f':
        {
            json_file.get(current_character);
            if (current_character != 'a') error("Wrong entry, may be going for false as first char was f");
            json_file.get(current_character);
            if (current_character != 'l') error("Wrong entry, may be going for false as first char was f");
            json_file.get(current_character);
            if (current_character != 's') error("Wrong entry, may be going for false as first char was f");
            json_file.get(current_character);
            if (current_character != 'e') error("Wrong entry, may be going for false as first char was f");

            JsonValueTypeSPtr value = make_shared_pointer(JsonValueType);
            value->type = JT_BOOL;
            value->value = false;

            if (!b_is_in_array) 
            {
                if (current_object->key_value_pair.count(current_key)) error("Found duplicate key here");
                current_object->key_value_pair.insert({current_key, value});
            }
            else 
            {
                current_array->array.push_back(value);
            }

            previous_state = next_state;
            next_state = FIND_NEXT_VALUE_OR_CLOSURE;
            break;
        }

        case 'n': 
        {
            json_file.get(current_character);
            if (current_character != 'u') error("Wrong entry, may be going for true as  char was t");
            json_file.get(current_character);
            if (current_character != 'l') error("Wrong entry, may be going for true as  char was r");
            json_file.get(current_character);
            if (current_character != 'l') error("Wrong entry, may be going for true as  char was u");   

            JsonValueTypeSPtr value = make_shared_pointer(JsonValueType);
            value->type = JT_NULL;
            value->value = nullptr;

            if (!b_is_in_array) 
            {
                if (current_object->key_value_pair.count(current_key)) error("Found duplicate key here");
                current_object->key_value_pair.insert({current_key, value});
            }
            else 
            {
                current_array->array.push_back(value);
            }

            previous_state = next_state;
            next_state = FIND_NEXT_VALUE_OR_CLOSURE;
            break;
        }
        default: {
            std::stringstream ss;
            ss << "Wrong input type for value. Current char: ";
            ss << current_character;
            ss << " ";
            ss << current_key;
            error(ss.str());
        }
            break;
    }
}

void Json::find_next_value_or_closure()
{
    while (json_file.get(current_character))
    {
        SKIP_CHARACTERS(current_character, current_line_number);
        break;
    }
    if (json_file.eof()){
        eof_reached = true;
        return;
    }
    switch (current_character)
    {
    case ',':
    {
        if (!b_is_in_array) 
        {
            previous_state = next_state;
            next_state = GET_KEY;
        }
        else
        {
            previous_state = next_state;
            next_state = GET_VALUE;
        }
    }
        break;

    case '}':
    {
        if (checker["{"] <= 0) error("Extra } in file");
        auto parent = current_object->parent;
        if (current_object->parent_type == JPT_OBJECT) {
            auto parent_object = std::get<JsonObjectSPtr>(parent);
            current_object = parent_object;
            b_is_in_array = false;
        }
        else {
            auto parent_array = std::get<JsonArraySPtr>(parent);
            current_array = parent_array;
            b_is_in_array = true;
        }
        previous_state = next_state;
        next_state = FIND_NEXT_VALUE_OR_CLOSURE;
        checker["{"]--;
        break;
    }

    case ']':
    {
        if (checker["["] <= 0) error("Extra ] in file");
        auto parent = current_array->parent;
        if (current_array->parent_type == JPT_OBJECT) {
            auto parent_object = std::get<JsonObjectSPtr>(parent);
            current_object = parent_object;
            b_is_in_array = false;
        }
        else {
            auto parent_array = std::get<JsonArraySPtr>(parent);
            current_array = parent_array;
            b_is_in_array = true;
        }
        previous_state = next_state;
        next_state = FIND_NEXT_VALUE_OR_CLOSURE;
        checker["["]--;
        break;
    }
    
    default: {
        std::stringstream ss;
        ss << "Find character " << current_character << " which should not be here";
        error(ss.str());
    }
        
        break;
    }
    
    
}

void Json::error(const std::string &msg)
{
    std::stringstream ss;
    ss << msg;
    ss << " at line: ";
    ss << current_line_number;
    throw std::runtime_error(ss.str());
}
