# JSON Parser

This project is a JSON parser implemented in C++. It provides support for handling JSON objects and arrays using smart pointers, making it easy to manage memory and avoid leaks. The parser allows accessing and manipulating JSON data in a type-safe manner using templates.

## Features

- **JsonObjectSPtr**: A shared pointer type used to store a JSON object.
- **JsonArraySPtr**: A shared pointer type used to store a JSON array.
- **Type-Safe Access**: Use `get<type>` template function to retrieve object values and `get<type>(index)` to access array elements.
- **Array Size Query**: Use `size()` method to get the number of elements in a JSON array.
- **Error Handling**: If there is any error in the JSON file, the parser will throw a `std::runtime_error` with a meaningful error message. You can also use the `error(msg)` function in the JSON class to check for specific errors and handle them appropriately.

## Usage

### Handling JSON Objects and Arrays

The JSON parser makes it easy to parse JSON from a file and work with JSON objects and arrays. The following example demonstrates how to parse JSON, access JSON properties, and retrieve values from arrays.

### Example Code

```cpp
#include <iostream>
#include <filesystem>
#include "JsonParser.h"  // Include your JSON parser header

int main() {
    try {
        // Path to JSON file
        std::filesystem::path path = std::filesystem::current_path();
        path /= "sample2.json";
        
        // Parse JSON file into JsonObjectSPtr
        JsonObjectSPtr root = JsonHelper::parse_json(path.c_str());

        // Get a JSON property that is an array
        JsonArraySPtr array = root->get<JsonArraySPtr>("array");
        JsonObjectSPtr obj2 = root->get<JsonObjectSPtr>("second");

        // Get a value from inside the array
        auto val = array->get<std::string>(5);
        std::cout << val << " \n";

        // Parse another JSON file
        path = std::filesystem::current_path();
        path /= "sample1.json";
        JsonObjectSPtr root_obj = JsonHelper::parse_json(path.c_str());

        // Get a JSON object property
        JsonObjectSPtr obj_1 = root_obj->get<JsonObjectSPtr>("project");
        auto id = obj_1->get<std::string>("id");
        std::cout << id << "\n";

        // Chain calls together to get a nested value inside the JSON
        auto obj_2_id = obj_1->get<JsonArraySPtr>("team")->get<JsonObjectSPtr>(0)->get<std::string>("id");
        std::cout << obj_2_id << "\n";
        
    } catch (const std::runtime_error& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
    }

    return 0;
}
