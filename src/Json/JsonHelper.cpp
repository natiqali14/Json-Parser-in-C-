#include "JsonHelper.h"
#include <fstream>
#include <sstream>

JsonObjectSPtr JsonHelper::parse_json(const std::string &file_path)
{
    auto pos = file_path.rfind('.');
    std::string str_to_compare = "json";
    if (pos != std::string::npos) {
        if (file_path.length() >= pos + 1 +4
        && file_path.compare(pos+1, str_to_compare.length(),  str_to_compare) == 0)
        {
            std::unique_ptr<Json> json = std::make_unique<Json>(file_path);
           
            auto root =  json->parse();
            json->reset_parent(root);
            return root;
        }
    }

    throw std::runtime_error("Not a json file");
    
}
