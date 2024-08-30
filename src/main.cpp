
#include <iostream>
#include <fstream>
#include <sstream>
#include <Json/JsonHelper.h>
#include <variant>
#include <string>
#include <filesystem>
/**
 * JsonObjecSPtr -> use this type to save a Json object as a shared ptr.
 * For can use object values using template function get<type>.
 * JsonArraySPtr -> This is use to hold json array as a value. This is also a shared Ptr
 * Also for array you can use get<type>(index) to get array values.
 * You can also query size() method to get arrays size
 */
int main() {
    
    // how to get json
    std::filesystem::path path = std::filesystem::current_path();
    path /= "sample2.json";
    JsonObjectSPtr root = JsonHelper::parse_json(path.c_str());
    
    // example to get a json property which is a array
    JsonArraySPtr array = root->get<JsonArraySPtr>("array");
    JsonObjectSPtr obj2 = root->get<JsonObjectSPtr>("second");
  
    // example to get a json value inside a array
    auto val = array->get<std::string>(5);
    std::cout << val << " \n";
    
    path = std::filesystem::current_path();
    path /= "sample1.json";
    JsonObjectSPtr root_obj = JsonHelper::parse_json(path.c_str());

    // example to get a json object
    JsonObjectSPtr obj_1 = root_obj->get<JsonObjectSPtr>("project");
    auto id = obj_1->get<std::string>("id");
    std::cout << id << "\n";

    // chaing together to get a value inside a json
    auto obj_2_id = obj_1->get<JsonArraySPtr>("team")->get<JsonObjectSPtr>(0)->get<std::string>("id");
    std::cout << obj_2_id << "\n";

    return 0;
}