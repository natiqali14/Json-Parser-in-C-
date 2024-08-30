#pragma once
#include <defines.h>
#include <string>
#include <Json/JsonObject.h>

namespace JsonHelper {
    /**
     * @brief Main function to parse json
     * 
     * @param file_path : path to json file
     * @return JsonObjectSPtr return shared pointer to root object
     */
    JsonObjectSPtr parse_json(const std::string& file_path);
}