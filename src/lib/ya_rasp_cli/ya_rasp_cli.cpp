#include "ya_rasp_cli.hpp"
#include "cpr/api.h"
#include "cpr/cprtypes.h"
#include "cpr/response.h"
#include "nlohmann/json_fwd.hpp"

#include <istream>
#include <sstream>
#include <string>
#include <string_view>
#include <fstream>

#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

namespace waybuilder {

const nlohmann::json::json_pointer YaRaspCli::kApiKeyJsonPtr{"api_key"};
const nlohmann::json::json_pointer YaRaspCli::kPointListPathJsonPtr{"point_list_path"};
const nlohmann::json::json_pointer YaRaspCli::kApiUrlJsonPtr{"api_url"};
const nlohmann::json::json_pointer YaRaspCli::kApiVersionJsonPtr{"api_version"};


YaRaspCli::YaRaspCli(std::string api_key, std::string point_list_path, std::string api_cfg_path)
 : api_key_{api_key}, point_list_path_{point_list_path}, api_cfg_path_{api_cfg_path} {
    std::ifstream point_list_file{point_list_path};
    if (point_list_file.is_open()) {
        point_list_ = nlohmann::json::parse(point_list_file);
    }
}


YaRaspCli::YaRaspCli(std::string api_cfg_path) : api_cfg_path_(api_cfg_path) {
    LoadCfg();
}


cpr::Response YaRaspCli::ScanPoints(std::string lang) {
    static std::string_view get_stations_url = "station_list";

    cpr::Response resp = cpr::Get(
        cpr::Url{
            BuildRequest(get_stations_url, {{"lang", lang}})
        }
    );

    if (resp.status_code == 200) {
        point_list_ = nlohmann::json::parse(resp.text);
    }

    return resp;
}


bool YaRaspCli::DumpCfg() {
    nlohmann::json api_cfg_json;
    
    api_cfg_json[kApiKeyJsonPtr] = api_key_; 
    api_cfg_json[kPointListPathJsonPtr] = point_list_path_;
    api_cfg_json[kApiUrlJsonPtr] = api_url_;
    api_cfg_json[kApiVersionJsonPtr] = api_version_; 
    
    std::ofstream api_cfg_file{api_cfg_path_};

    if (!api_cfg_file.is_open())
        return false;

    api_cfg_file << api_cfg_json;
    return true;
}


bool YaRaspCli::LoadCfg() {
    std::ifstream api_cfg_file{api_cfg_path_};

    if (!api_cfg_file.is_open())
        return false;

    nlohmann::json api_cfg_json = nlohmann::json::parse(api_cfg_file);

    api_key_ = api_cfg_json.at(kApiKeyJsonPtr);
    point_list_path_ = api_cfg_json.at(kPointListPathJsonPtr);
    api_url_ = api_cfg_json.at(kApiUrlJsonPtr);
    api_version_ = api_cfg_json.at(kApiVersionJsonPtr);

    
    std::ifstream point_list_file{point_list_path_};
    if (point_list_file.is_open()) {
        point_list_ = nlohmann::json::parse(point_list_file);
    }

    return true;
}


bool YaRaspCli::Save() {
    bool save_state = DumpCfg();

    std::ofstream point_list_file{point_list_path_};

    save_state = save_state && point_list_file.is_open();

    if (point_list_file.is_open()) {
        point_list_file << point_list_;
    }

    return save_state;
}


std::string YaRaspCli::BuildRequest(std::string_view req_str, 
  std::initializer_list<std::pair<std::string_view, std::string_view>> args) {
    std::stringstream request_stream;

    request_stream << api_url_ << '/' << api_version_ << '/';
    request_stream << req_str << '/' << '?';
    request_stream << "api_key" << '=' << api_key_;

    for (auto&& arg : args) {
        request_stream << '&' << arg.first << '=' << arg.second;
    }

    return request_stream.str();
};


} // namespace waybuilder