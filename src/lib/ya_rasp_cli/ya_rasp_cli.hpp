#ifndef _YA_RASP_CLI_HPP_
#define _YA_RASP_CLI_HPP_

#include <initializer_list>
#include <string>
#include <optional>
#include <functional>
#include <unordered_map>

#include <nlohmann/json.hpp>
#include <cpr/cpr.h>
#include <boost/log/sources/logger.hpp>

namespace waybuilder {

class YaRaspCli {
 public:
    YaRaspCli(const std::string& api_key, const std::string& point_list_path,
        const std::string& api_cfg_path, const std::string& log_dir_path = "./logs/");
    YaRaspCli(const std::string& api_cfg_path, const std::string& log_dir_path = "./logs/");

 public:
    cpr::Response ScanPoints(std::string lang = "ru_RU");

    std::optional<std::reference_wrapper<nlohmann::json>> CountryList();
    std::optional<std::reference_wrapper<nlohmann::json>> RegionList(const std::string& country_id);
    std::optional<std::reference_wrapper<nlohmann::json>>
      CityList(const std::string& country_id, const std::string& region_id);
    std::optional<std::reference_wrapper<nlohmann::json>>
      StationList(const std::string& country_id, const std::string& region_id, const std::string& city_id);

 public:
    nlohmann::json FindCountry(const std::string& name);
    nlohmann::json FindRegion(const std::string& name);
    nlohmann::json FindCity(const std::string& name);
    nlohmann::json FindStation(const std::string& name);

 public: 
 bool PointsJsonOutput(std::ostream& stream, const nlohmann::json& points_json,
                    const std::string& name_colom, const std::string& id_colom);
 
 public:
    bool DumpCfg();
    bool LoadCfg();

 public:
    bool Save();

 public:
    boost::log::sources::logger& GetLoggerRef() { return logger_; };

 private:
    std::string BuildRequest(std::string_view req_str, 
      std::initializer_list<std::pair<std::string_view, std::string_view>> args);

    void LogConfigurate(const std::string& log_dir_path);

 private:
    boost::log::sources::logger logger_;

 private:
    std::string api_key_;
    std::string api_url_;
    std::string api_version_;

    nlohmann::json point_list_;

 private: 
    std::string api_cfg_path_;
    std::string point_list_path_;

 public:
    static const nlohmann::json::json_pointer kPointIdJsonPtr;
    static const nlohmann::json::json_pointer kPointNameJsonPtr;

 private:
    static const nlohmann::json::json_pointer kApiKeyJsonPtr;
    static const nlohmann::json::json_pointer kPointListPathJsonPtr;
    static const nlohmann::json::json_pointer kApiUrlJsonPtr;
    static const nlohmann::json::json_pointer kApiVersionJsonPtr;

 private:
    static const nlohmann::json::json_pointer kCountryJsonPtr;
    static const nlohmann::json::json_pointer kRegionJsonPtr;
    static const nlohmann::json::json_pointer kCityJsonPtr;
    static const nlohmann::json::json_pointer kStationJsonPtr;
};

} // namespace waybuilder

#endif // _YA_RASP_CLI_HPP_