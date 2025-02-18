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

const nlohmann::json::json_pointer YaRaspCli::kCountryJsonPtr{"/countries"};
const nlohmann::json::json_pointer YaRaspCli::kRegionJsonPtr{"/regions"};
const nlohmann::json::json_pointer YaRaspCli::kCityJsonPtr{"/settlements"};
const nlohmann::json::json_pointer YaRaspCli::kStationJsonPtr{"/stations"};

const nlohmann::json::json_pointer YaRaspCli::kPointIdJsonPtr{"/codes/yandex_code"};
const nlohmann::json::json_pointer YaRaspCli::kPointNameJsonPtr{"/title"};


YaRaspCli::YaRaspCli(const std::string& api_key, const std::string& point_list_path,
    const std::string& api_cfg_path, const std::string& log_dir_path)
 : api_key_{api_key}, point_list_path_{point_list_path}, api_cfg_path_{api_cfg_path} {
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


cpr::Response YaRaspCli::ScanPoints(std::string lang) {
    static std::string_view get_stations_url = "stations_list";

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
    request_stream << "apikey" << '=' << api_key_;

    for (auto&& arg : args) {
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
        auto region_json_itr 
            = std::find_if(country_list.begin(), country_list.end(),
            [&](auto& country){
            return country.at(kPointIdJsonPtr).template get<std::string>() == country_id;
        });

        if (region_json_itr == country_list.end())
            return {};

        return *region_json_itr;
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

        auto city_json_itr 
            = std::find_if(region_list.begin(), region_list.end(),
            [&](auto& region){
            return region.at(kPointIdJsonPtr).template get<std::string>() == region_id;
        });
        if (city_json_itr == region_list.end())
            return {};

        return *city_json_itr;
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
        auto station_json_itr 
            = std::find_if(city_list.begin(), city_list.end(),
                [&](auto& city){
            return city.at(kPointIdJsonPtr).template get<std::string>() == city_id;
        });

        if (station_json_itr == city_list.end())
            return {};

        return *station_json_itr;
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


nlohmann::json YaRaspCli::FindCountry(const std::string& name) {
    nlohmann::json country_list = nlohmann::json::array();

    try {
        for (auto& coutry : point_list_.at(kCountryJsonPtr)) {
            if (coutry.contains(kPointIdJsonPtr) && coutry.contains(kPointNameJsonPtr)) {
                if (coutry.at(kPointNameJsonPtr).get_ref<std::string&>().find(name) != std::string::npos) {
                    nlohmann::json coutry_json = {};
                    
                    BuildJsonPath(coutry_json, kPointIdJsonPtr);
                    BuildJsonPath(coutry_json, kPointNameJsonPtr);
                    
                    coutry_json.at(kPointIdJsonPtr) = coutry.at(kPointIdJsonPtr);
                    coutry_json.at(kPointNameJsonPtr) = coutry.at(kPointNameJsonPtr);

                    country_list.push_back(coutry_json);
                }
            }
        }
    } catch (nlohmann::json::out_of_range& ex) {
        BOOST_LOG_SEV(GetLoggerRef(), boost::log::trivial::error) << "exception id: " << ex.id << " | " << ex.what();
    } catch (nlohmann::json::parse_error& ex) {
        BOOST_LOG_SEV(GetLoggerRef(), boost::log::trivial::error) << "exception id: " << ex.id << " | " << ex.what();
    }

    return country_list;
}


nlohmann::json YaRaspCli::FindRegion(const std::string& name) {

}


nlohmann::json YaRaspCli::FindCity(const std::string& name) {

}


nlohmann::json FindStation(const std::string& name) {

}


bool YaRaspCli::PointsJsonOutput(std::ostream& stream, const nlohmann::json& points_json,
    const std::string& name_colom, const std::string& id_colom) {
    static constexpr size_t kCollomSpaceOffset = 20;

    if (points_json.empty()) {
        BOOST_LOG_SEV(GetLoggerRef(), boost::log::trivial::error) 
            << "output point error, list of points is empty"; 
        return false;
    }

    std::cout << "country name" << " " << std::setw(kCollomSpaceOffset) << "country_id" << std::endl;

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
            << "find country error" << " | " 
            << "exception id: " << ex.id << " | "
            << ex.what();
    }
    return true;
};

#if 0

std::unordered_map<std::string, std::string> YaRaspCli::CountryList() {
    std::unordered_map<std::string, std::string> country_list;

    for (auto&& country : point_list_.at(kCountryJsonPtr).items()) {
        country_list.insert(
            {
                country.value().at(kPointIdJsonPtr).get<std::string>(),
                country.value().at(kPointNameJsonPtr).get<std::string>()
            }
        );
    }

    return country_list;
}


std::unordered_map<std::string, std::string> YaRaspCli::CityList(const std::string& country_id) {
    std::unordered_map<std::string, std::string> city_list;
    
    auto coutry_json 
        = std::find_if(point_list_.at(kCountryJsonPtr).begin(), point_list_.at(kCountryJsonPtr).end(),
            [&](auto& country){
        return country.at(kPointIdJsonPtr).template get<std::string>() == country_id;
    });

    for (auto&& city : coutry_json->at(kCityJsonPtr).items()) {
        city_list.insert(
            {
                city.value().at(kPointIdJsonPtr).get<std::string>(),
                city.value().at(kPointNameJsonPtr).get<std::string>()
            }
        );
    }

    return city_list;
}


std::unordered_map<std::string, std::string> YaRaspCli::StationList(const std::string& country_id, const std::string& city_id) {
    std::unordered_map<std::string, std::string> city_list;
    
    auto coutry_json  = std::find_if(
        point_list_.at(kCountryJsonPtr).begin(), point_list_.at(kCountryJsonPtr).end(),
            [&](auto& country){
        return country.at(kPointIdJsonPtr).template get<std::string>() == country_id;
    });

    auto city_json = std::find_if(
        coutry_json->at(kCityJsonPtr).begin(), coutry_json->at(kCityJsonPtr).end(),
            [&](auto& city){
        return city.at(kPointIdJsonPtr).template get<std::string>() == city_id;
    });

    for (auto&& country : city_json->at(kCityJsonPtr).items()) {
        city_list.insert(
            {
                country.value().at(kPointIdJsonPtr).get<std::string>(),
                country.value().at(kPointNameJsonPtr).get<std::string>()
            }
        );
    }

    return city_list;
}

#endif

} // namespace waybuilder