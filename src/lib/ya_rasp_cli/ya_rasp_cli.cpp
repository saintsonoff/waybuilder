#include "ya_rasp_cli.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <functional>
#include <sstream>
#include <span>
#include <type_traits>
#include <string>
#include <string_view>
#include <optional>
#include <ostream>

#include <nlohmann/json.hpp>
#include <nlohmann/detail/json_pointer.hpp>
#include <nlohmann/json_fwd.hpp>

#include <cpr/cpr.h>
#include <cpr/api.h>
#include <cpr/cprtypes.h>
#include <cpr/response.h>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/sources/severity_feature.hpp>

namespace waybuilder {

const nlohmann::json::json_pointer YaRaspCli::kApiKeyJsonPtr{"/api_key"};
const nlohmann::json::json_pointer YaRaspCli::kPointListPathJsonPtr{"/point_list_path"};
const nlohmann::json::json_pointer YaRaspCli::kApiUrlJsonPtr{"/api_url"};
const nlohmann::json::json_pointer YaRaspCli::kApiVersionJsonPtr{"/api_version"};
const nlohmann::json::json_pointer YaRaspCli::kApiLangJsonPtr{"/api_lang"};

const nlohmann::json::json_pointer YaRaspCli::kCountryJsonPtr{"/countries"};
const nlohmann::json::json_pointer YaRaspCli::kRegionJsonPtr{"/regions"};
const nlohmann::json::json_pointer YaRaspCli::kCityJsonPtr{"/settlements"};
const nlohmann::json::json_pointer YaRaspCli::kStationJsonPtr{"/stations"};

const nlohmann::json::json_pointer YaRaspCli::kPointIdJsonPtr{"/codes/yandex_code"};
const nlohmann::json::json_pointer YaRaspCli::kPointNameJsonPtr{"/title"};


YaRaspCli::YaRaspCli(const std::string& api_key, const std::string& point_list_path,
    const std::string& api_cfg_path, const std::string& api_lang, const std::string& log_dir_path)
 : api_key_{api_key}, point_list_path_{point_list_path}, api_cfg_path_{api_cfg_path}, api_lang_(api_lang) {
    std::ifstream point_list_file{point_list_path};
    if (point_list_file.is_open()) {
        point_list_ = nlohmann::json::parse(point_list_file);
    }

    LogConfigurate(log_dir_path);
}


YaRaspCli::YaRaspCli(const std::string& api_cfg_path, const std::string& log_dir_path) : api_cfg_path_(api_cfg_path) {
    LoadCfg();
    LogConfigurate(log_dir_path);
}


cpr::Response YaRaspCli::ScanPoints() {
    static const std::string_view kGetStationsUrl = "stations_list";

    cpr::Response resp = cpr::Get(
        cpr::Url{
            BuildRequest(kGetStationsUrl, {{"lang", api_lang_}})
        }
    );

    if (resp.status_code == 200) {
        point_list_ = nlohmann::json::parse(resp.text);
    }

    return resp;
}


cpr::Response YaRaspCli::ScanWays(const std::string& from_point, const std::string& to_point,
    const std::string& date, bool transfers, const std::string& transport_types, const std::string& system,
    const std::string& show_systems, size_t offset, size_t limit, bool add_days_mask,
    const std::string& result_timezone) {
    static const std::string_view kGetWaysUrl = "search";

    std::string offset_str{offset ? std::to_string(offset) : ""};
    std::string transfers_str{transfers ? std::to_string(transfers) : ""};
    std::string limit_str{limit ? std::to_string(offset) : ""};
    std::string add_days_mask_str{add_days_mask ? std::to_string(add_days_mask) : ""};

    cpr::Response resp = cpr::Get(
        cpr::Url{
            BuildRequest(kGetWaysUrl, {
                {"lang", api_lang_},
                {"from", from_point},
                {"to", to_point},
                {"date", date},
                {"transport_types", transport_types},
                {"system", system},
                {"show_systems", show_systems},
                {"offset", offset_str},
                {"limit", limit_str},
                {"add_days_mask", add_days_mask_str},
                {"result_timezone", result_timezone},
                {"transfers", transfers_str}
            })
        }
    );    
}


bool YaRaspCli::DumpCfg() {
    nlohmann::json api_cfg_json;
    
    api_cfg_json[kApiKeyJsonPtr] = api_key_; 
    api_cfg_json[kPointListPathJsonPtr] = point_list_path_;
    api_cfg_json[kApiUrlJsonPtr] = api_url_;
    api_cfg_json[kApiVersionJsonPtr] = api_version_; 
    api_cfg_json[kApiLangJsonPtr] = api_lang_; 
    
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

    if (api_cfg_json.contains(kApiKeyJsonPtr)
      && api_cfg_json.contains(kApiUrlJsonPtr)
      && api_cfg_json.contains(kApiVersionJsonPtr)) {
        api_key_ = api_cfg_json.at(kApiKeyJsonPtr);
        api_url_ = api_cfg_json.at(kApiUrlJsonPtr);
        api_version_ = api_cfg_json.at(kApiVersionJsonPtr);
    } else {
        BOOST_LOG_SEV(GetLoggerRef(), boost::log::trivial::error)
            << "api construction error" << " | " 
            << "can not counstruct yandex raspisania api by config file, fatal error" << " | "
            << "can not found important parameters";
            return false;
    }

    try {
        point_list_path_ = api_cfg_json.at(kPointListPathJsonPtr);
        api_lang_ = api_cfg_json.at(kApiLangJsonPtr);
    } catch (nlohmann::json::out_of_range& ex) {
        BOOST_LOG_SEV(GetLoggerRef(), boost::log::trivial::error) 
            << "api construction error" << " | " 
            << "can not counstruct yandex raspisania api by config file, will use default config" << " | " 
            << "exception id: " << ex.id << " | "
            << ex.what();
        point_list_path_ = "./point_list.json";
        api_lang_ = "ru_RU";
    }


    
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
    request_stream << "apikey" << '=' << api_key_;

    for (auto&& arg : args) {
        if (!arg.first.empty() && !arg.second.empty())
        request_stream << '&' << arg.first << '=' << arg.second;
    }

    return request_stream.str();
};


void YaRaspCli::LogConfigurate(const std::string& log_dir_path) {
    namespace logging = boost::log;
    namespace keywords = boost::log::keywords;
    namespace expr = boost::log::expressions;

    if (!std::filesystem::exists(log_dir_path)) {
        std::filesystem::create_directories(log_dir_path);
    }

    logging::add_file_log(
        keywords::file_name = log_dir_path + "/log_%Y-%m-%d_%H-%M-%S.log",
        keywords::rotation_size = 10 * 1024 * 1024,
        keywords::format = (
            expr::stream
                << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S")
                << " [" << logging::trivial::severity << "] "
                << expr::smessage
        )
    );

    logging::add_common_attributes();
};


std::optional<std::reference_wrapper<nlohmann::json>> YaRaspCli::CountryList() {
    if (point_list_.empty())
        return {};

    try {
        auto& country_list = point_list_.at(kCountryJsonPtr);
        return country_list;
    } catch (nlohmann::json::out_of_range& ex) {
        BOOST_LOG_SEV(GetLoggerRef(), boost::log::trivial::error) << "exception id: " << ex.id << " | " << ex.what();
        return {};
    }
}


std::optional<std::reference_wrapper<nlohmann::json>> YaRaspCli::RegionList(const std::string& country_id) {
    auto&& country_opt = CountryList();
    
    if (!country_opt)
        return {};

    nlohmann::json& country_list = country_opt->get();

    try {
        auto country_json_itr 
            = std::find_if(country_list.begin(), country_list.end(),
            [&](auto& country){
                if (country.contains(kPointIdJsonPtr)) {
                    return country.at(kPointIdJsonPtr).template get_ref<const std::string&>() == country_id;
                } else {
                    return false;
                }
        });

        if (country_json_itr == country_list.end())
            return {};

        return country_json_itr->at(kRegionJsonPtr);
    } catch (nlohmann::json::out_of_range& ex) {
        BOOST_LOG_SEV(GetLoggerRef(), boost::log::trivial::error) << "exception id: " << ex.id << " | " << ex.what();
        return {};
    }
}


std::optional<std::reference_wrapper<nlohmann::json>> 
  YaRaspCli::CityList(const std::string& country_id, const std::string& region_id) {
    auto&& region_opt = RegionList(country_id);
    
    if (!region_opt)
        return {};

    nlohmann::json& region_list = region_opt->get();

    try {
        auto region_json_itr 
            = std::find_if(region_list.begin(), region_list.end(),
            [&](auto& region){
            if (region.contains(kPointIdJsonPtr)) {
                return region.at(kPointIdJsonPtr).template get_ref<const std::string&>() == region_id;
            } else {
                return false;
            }
        });
        if (region_json_itr == region_list.end())
            return {};

        return region_json_itr->at(kCityJsonPtr);
    } catch (nlohmann::json::out_of_range& ex) {
        BOOST_LOG_SEV(GetLoggerRef(), boost::log::trivial::error) << "exception id: " << ex.id << " | " << ex.what();
        return {};
    }

}


std::optional<std::reference_wrapper<nlohmann::json>>
  YaRaspCli::StationList(const std::string& country_id, const std::string& region_id, const std::string& city_id) {
    auto&& city_opt = CityList(country_id, region_id);
    
    if (!city_opt)
        return {};

    nlohmann::json& city_list = city_opt->get();

    try {
        auto city_json_itr 
            = std::find_if(city_list.begin(), city_list.end(),
                [&](auto& city){
                    if (city.contains(kPointIdJsonPtr)) {
                        return city.at(kPointIdJsonPtr).template get_ref<const std::string&>() == city_id;
                    } else {
                        return false;
                    }
        
        });

        if (city_json_itr == city_list.end())
            return {};

        return city_json_itr->at(kStationJsonPtr);
    } catch (nlohmann::json::out_of_range& ex) {
        BOOST_LOG_SEV(GetLoggerRef(), boost::log::trivial::error) << "exception id: " << ex.id << " | " << ex.what();
        return {};
    }
}


namespace {

template<typename RefStringType>
std::vector<typename nlohmann::json_pointer<RefStringType>::string_t>
    JsonPtrToVectorOfString(const nlohmann::json_pointer<RefStringType>& json_ptr) {
    using string_type = typename nlohmann::json_pointer<RefStringType>::string_t;

    std::vector<string_type> path_container;
    std::stringstream ss_buff(json_ptr.to_string());

    string_type path_buff;
    while (std::getline(ss_buff, path_buff, '/')) {
        path_container.push_back(path_buff);
    }

    return path_container;
}


void BuildJsonPath(nlohmann::json& json_object, const std::decay_t<decltype(json_object)>::json_pointer& path_ptr) {
    auto path_cont = JsonPtrToVectorOfString(path_ptr);

    if (path_cont.empty())
        return;

    nlohmann::json::json_pointer point_json{""};

    for (auto&& path : std::span{path_cont.begin() + 1, path_cont.end()}) {
        if (!json_object.contains(point_json / path)) {
            json_object[point_json][path] = {};
        }
        point_json /= path;
    }
}

} // namespace


nlohmann::json YaRaspCli::FindPointByName(const nlohmann::json& point_list, const std::string& name) {
    nlohmann::json result_point_list = nlohmann::json::array();

    try {
        for (auto& coutry : point_list) {
            if (coutry.contains(kPointIdJsonPtr) && coutry.contains(kPointNameJsonPtr)) {
                if (coutry.at(kPointNameJsonPtr).get_ref<const std::string&>().find(name) != std::string::npos) {
                    nlohmann::json point_json = {};
                    
                    BuildJsonPath(point_json, kPointIdJsonPtr);
                    BuildJsonPath(point_json, kPointNameJsonPtr);

                    point_json.at(kPointIdJsonPtr) = coutry.at(kPointIdJsonPtr);
                    point_json.at(kPointNameJsonPtr) = coutry.at(kPointNameJsonPtr);

                    result_point_list.push_back(point_json);
                }
            }
        }
    } catch (nlohmann::json::out_of_range& ex) {
        BOOST_LOG_SEV(GetLoggerRef(), boost::log::trivial::error) << "exception id: " << ex.id << " | " << ex.what();
    } catch (nlohmann::json::parse_error& ex) {
        BOOST_LOG_SEV(GetLoggerRef(), boost::log::trivial::error) << "exception id: " << ex.id << " | " << ex.what();
    }

    return result_point_list;
};


nlohmann::json YaRaspCli::FindCountry(const std::string& name) {
    try {
        return FindPointByName(point_list_.at(kCountryJsonPtr), name);
    } catch (nlohmann::json::out_of_range& ex) {
        BOOST_LOG_SEV(GetLoggerRef(), boost::log::trivial::error) << "exception id: " << ex.id << " | " << ex.what();
    } catch (nlohmann::json::parse_error& ex) {
        BOOST_LOG_SEV(GetLoggerRef(), boost::log::trivial::error) << "exception id: " << ex.id << " | " << ex.what();
    }
    return {};
}


nlohmann::json YaRaspCli::FindRegion(const std::string& name) {
    nlohmann::json result = nlohmann::json::array();
    try {
        auto& coutries = point_list_.at(kCountryJsonPtr);
        std::for_each(coutries.begin(), coutries.end(), [&](auto& coutry){
            nlohmann::json region_list = FindPointByName(coutry.at(kRegionJsonPtr), name);
            result.insert(result.end(), region_list.begin(), region_list.end());
        });
    } catch (nlohmann::json::out_of_range& ex) {
        BOOST_LOG_SEV(GetLoggerRef(), boost::log::trivial::error) << "exception id: " << ex.id << " | " << ex.what();
    } catch (nlohmann::json::parse_error& ex) {
        BOOST_LOG_SEV(GetLoggerRef(), boost::log::trivial::error) << "exception id: " << ex.id << " | " << ex.what();
    }
    return result;
}


nlohmann::json YaRaspCli::FindCity(const std::string& name) {
    nlohmann::json result = nlohmann::json::array();
    try {
        auto& coutries = point_list_.at(kCountryJsonPtr);
        std::for_each(coutries.begin(), coutries.end(), [&](auto& coutry){
            auto& regions = coutry.at(kRegionJsonPtr);
            std::for_each(regions.begin(), regions.end(), [&](auto& region){
                nlohmann::json city_list = FindPointByName(region.at(kCityJsonPtr), name);
                result.insert(result.end(), city_list.begin(), city_list.end());
            });
        });
    } catch (nlohmann::json::out_of_range& ex) {
        BOOST_LOG_SEV(GetLoggerRef(), boost::log::trivial::error) << "exception id: " << ex.id << " | " << ex.what();
    } catch (nlohmann::json::parse_error& ex) {
        BOOST_LOG_SEV(GetLoggerRef(), boost::log::trivial::error) << "exception id: " << ex.id << " | " << ex.what();
    }
    return result;
}


nlohmann::json YaRaspCli::FindStation(const std::string& name) {
    nlohmann::json result = nlohmann::json::array();
    try {
        auto& coutries = point_list_.at(kCountryJsonPtr);
        std::for_each(coutries.begin(), coutries.end(), [&](auto& coutry){
            auto& regions = coutry.at(kRegionJsonPtr);
            std::for_each(regions.begin(), regions.end(), [&](auto& region){
                auto& cities = region.at(kCityJsonPtr);
                std::for_each(cities.begin(), cities.end(), [&](auto& city){
                    nlohmann::json station_list = FindPointByName(city.at(kStationJsonPtr), name);
                    result.insert(result.end(), station_list.begin(), station_list.end());
                });
            });
        });
    } catch (nlohmann::json::out_of_range& ex) {
        BOOST_LOG_SEV(GetLoggerRef(), boost::log::trivial::error) << "exception id: " << ex.id << " | " << ex.what();
    } catch (nlohmann::json::parse_error& ex) {
        BOOST_LOG_SEV(GetLoggerRef(), boost::log::trivial::error) << "exception id: " << ex.id << " | " << ex.what();
    }
    return result;
}


bool YaRaspCli::PointsJsonOutput(std::ostream& stream, const nlohmann::json& points_json,
    const std::string& name_colom, const std::string& id_colom) {
    static constexpr size_t kCollomSpaceOffset = 20;

    if (points_json.empty()) {
        BOOST_LOG_SEV(GetLoggerRef(), boost::log::trivial::error) 
            << "output point error, list of points is empty"; 
        return false;
    }

    stream << name_colom << " " << std::setw(kCollomSpaceOffset) << id_colom << std::endl;

    try {
        for (auto&& point : points_json) {
            if (point.contains(kPointNameJsonPtr) && point.contains(kPointIdJsonPtr)) {
                stream
                    << point.at(kPointNameJsonPtr).get<std::string>() << std::setw(kCollomSpaceOffset)
                    << point.at(kPointIdJsonPtr).get<std::string>() << std::endl;
            }
        }
    } catch (nlohmann::json::out_of_range& ex) {
        BOOST_LOG_SEV(GetLoggerRef(), boost::log::trivial::error) 
            << "output point error" << " | " 
            << "exception id: " << ex.id << " | "
            << ex.what();
    }
    return true;
};

} // namespace waybuilder