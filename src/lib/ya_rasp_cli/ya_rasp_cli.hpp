#ifndef _YA_RASP_CLI_HPP_
#define _YA_RASP_CLI_HPP_

#include <initializer_list>
#include <optional>
#include <string>
#include <fstream>
#include <cstdint>

#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

namespace waybuilder {

class YaRaspCli {
 public:
    YaRaspCli(std::string api_key, std::string point_list_path, std::string api_cfg_path);
    YaRaspCli(std::string api_cfg_path);

 public:
    cpr::Response ScanPoints(std::string lang);


 public:
    bool DumpCfg();
    bool LoadCfg();

 public:
    bool Save();

 private:
    std::string BuildRequest(std::string_view req_str, 
      std::initializer_list<std::pair<std::string_view, std::string_view>> args);

 private:
    std::string api_key_;
    std::string api_url_;
    std::string api_version_;

    nlohmann::json point_list_;

 private: 
    std::string api_cfg_path_;
    std::string point_list_path_;

 private:
    static const nlohmann::json::json_pointer kApiKeyJsonPtr;
    static const nlohmann::json::json_pointer kPointListPathJsonPtr;
    static const nlohmann::json::json_pointer kApiUrlJsonPtr;
    static const nlohmann::json::json_pointer kApiVersionJsonPtr;
};

} // namespace waybuilder

#endif // _YA_RASP_CLI_HPP_