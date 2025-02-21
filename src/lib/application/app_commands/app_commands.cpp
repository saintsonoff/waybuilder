#include "app_commands.hpp"
#include "nlohmann/json_fwd.hpp"


#include <iomanip>
#include <string_view>
#include <memory>
#include <iostream>

#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/trivial.hpp>

#include <nlohmann/json.hpp>

namespace waybuilder {

namespace commands {

std::string_view Help::kHelpText = "help command";


CommandExeStatus Save::Run() {
    if (cli_.Save()) {
        output_manager_.GetStreamRef() << "Save is done" << std::endl;
    } else {
        output_manager_.GetStreamRef() << "Cannot save config or points, check path" << std::endl;
    }

    return CommandExeStatus::CORRECT;
}


CommandExeStatus ChangeLang::Run() {
    std::string new_lang;
    std::cin >> new_lang;

    std::string old_lang = cli_.GetLang();
    cli_.SetLang(new_lang);
    if (cli_.ScanPoints().status_code != 200) {
        output_manager_.GetStreamRef() << "Unknowing lang, check yandex langs table" << "\n"
            << "Actual language: " << old_lang << std::endl;
        cli_.SetLang(old_lang);
    }

    output_manager_.GetStreamRef() << "Change language is done" << std::endl;

    return CommandExeStatus::CORRECT;
};


CommandExeStatus ScanPoints::Run() {
    auto resp = cli_.ScanPoints();

    if (resp.status_code == 200) {
        output_manager_.GetStreamRef() << "Scan is done" << std::endl;
    } else {
        output_manager_.GetStreamRef() << "Cannot scan points, check api config or log journal" << std::endl;

        BOOST_LOG_SEV(cli_.GetLoggerRef(), boost::log::trivial::error)
            << "api request error" << " | " 
            << "response code: " <<  resp.status_code << " | "
            << "reason: " <<  resp.reason << " | "
            << "request url: " <<  resp.url << " | "
            << "text: " <<  resp.text;
    }

    return CommandExeStatus::CORRECT;
}


CommandExeStatus ScanWays::Run() {
    auto resp = cli_.ScanPoints();

    if (resp.status_code == 200) {
        output_manager_.GetStreamRef() << "Scan is done" << std::endl;
    } else {
        output_manager_.GetStreamRef() << "Cannot scan points, check api config or log journal" << std::endl;
    }
    return CommandExeStatus::CORRECT;
}

   
CommandExeStatus ListCountry::Run() {
    auto&& country_list = cli_.CountryList();

    if (!output_manager_.PointsJsonOutput(cli_, country_list->get(), "country name", "coutry id")) {
        output_manager_.GetStreamRef() << "Get list error" << "\n"
            << "try to rescan points" << std::endl;
    }

    return CommandExeStatus::CORRECT;
}


CommandExeStatus ListRegion::Run() {
    std::string country_id;
    std::cin >> country_id;

    nlohmann::json list;
    if (country_id == kAllValue) {        
        list = cli_.FindRegion("");
    } else {
        auto&& optional_list = cli_.RegionList(country_id);

        if (optional_list) {
            list = optional_list->get();
        }

    }
    if (!output_manager_.PointsJsonOutput(cli_, list, "region name", "region id")) {
        output_manager_.GetStreamRef() << "Can not find region by {" << country_id << "} request" << "\n"
            << "try to rescan points" << std::endl;
    }


    return CommandExeStatus::CORRECT;
}


CommandExeStatus ListCity::Run() {
    std::string country_id;
    std::cin >> country_id;


    std::string region_id = "";

    nlohmann::json list;
    if (country_id == kAllValue) {        
        list = cli_.FindCity("");
    } else {
        std::cin >> region_id;
        auto&& optional_list = cli_.CityList(country_id, region_id);

        if (optional_list) {
            list = optional_list->get();
        }
    }

    if (!output_manager_.PointsJsonOutput(cli_, list, "city name", "city id")) {
        output_manager_.GetStreamRef() << "Can not find city by {" << country_id 
            << (region_id.empty() ? "" : " / ") << region_id
            << "} request" << "\n"
            << "try to rescan points" << std::endl;
    }

    return CommandExeStatus::CORRECT;
}


CommandExeStatus ListStation::Run() {
    std::string country_id;
    std::cin >> country_id;

    std::string region_id = "";
    std::string city_id = "";

    nlohmann::json list;
    if (country_id == kAllValue) {        
        list = cli_.FindStation("");
    } else {
        std::cin >> region_id >> city_id;
        auto&& optional_list = cli_.StationList(country_id, region_id, city_id);

        if (optional_list) {
            list = optional_list->get();
        }
    }

    if (!output_manager_.PointsJsonOutput(cli_, list, "station name", "station id")) {
        output_manager_.GetStreamRef() << "Can not find station by {" << country_id
            << (region_id.empty() ? "" : " / ") << region_id
            << (city_id.empty() ? "" : " / ") << city_id
            << "} request" << "\n"
            << "try to rescan points" << std::endl;
    }

    return CommandExeStatus::CORRECT;
}


CommandExeStatus ListWay::Run() {
    std::string from_point_id;
    std::string to_point_id;
    std::string date;

    std::cin >> from_point_id >> to_point_id >> date;

    auto resp = cli_.ScanWays(from_point_id, to_point_id, date);

    if (resp.status_code != 200) {
        output_manager_.GetStreamRef() << "Ways scan error, check log journal" << std::endl;
        return CommandExeStatus::CORRECT;
    }
    
    nlohmann::json ways_json;
    try {
        ways_json = nlohmann::json::parse(resp.text);
    } catch (nlohmann::json::parse_error& ex) {
        BOOST_LOG_SEV(cli_.GetLoggerRef(), boost::log::trivial::error)
            << "parse ways json error " << " | "
            << "id: " << ex.id << " | "
            << "text: " << ex.what();
        output_manager_.GetStreamRef() << "Ways scan error, check log journal" << std::endl;
        return CommandExeStatus::CORRECT;
    }

    if (!output_manager_.WaysJsonOutput(cli_, ways_json)) {
        output_manager_.GetStreamRef() << "Can not find ways by {"
            << (from_point_id.empty() ? "" : " from point:  " + from_point_id + " / ") 
            << (to_point_id.empty() ? "" : " to point:  " + to_point_id + " / ") 
            << (date.empty() ? "" : " date: " + date) 
            << "} request" << "\n"
            << "try to rescan points" << std::endl;
    }        

    return CommandExeStatus::CORRECT;
}


CommandExeStatus FindCountry::Run() {
    std::string search_str;
    std::cin >> search_str;

    if (search_str == kAllValue) {
        search_str = "";        
    }

    auto&& list = cli_.FindCountry(search_str);

    if (!output_manager_.PointsJsonOutput(cli_, list, "country name", "coutry id")) {
        output_manager_.GetStreamRef() << "Can not find country by {" << search_str << "} request" << "\n"
            << "try to rescan points" << std::endl;
    }

    return CommandExeStatus::CORRECT;
}


CommandExeStatus FindRegion::Run() {
    std::string search_str;
    std::cin >> search_str;

    if (search_str == kAllValue) {
        search_str = "";        
    }

    auto&& list = cli_.FindRegion(search_str);

    if (!output_manager_.PointsJsonOutput(cli_, list, "region name", "region id")) {
        output_manager_.GetStreamRef() << "Can not find region by {" << search_str << "} request" << "\n"
            << "try to rescan points" << std::endl;
    }

    return CommandExeStatus::CORRECT;
}


CommandExeStatus FindCity::Run() {
    std::string search_str;
    std::cin >> search_str;

    if (search_str == kAllValue) {
        search_str = "";        
    }

    auto&& list = cli_.FindCity(search_str);

    if (!output_manager_.PointsJsonOutput(cli_, list, "city name", "city id")) {
        output_manager_.GetStreamRef() << "Can not find city by {" << search_str << "} request" << "\n"
            << "try to rescan points" << std::endl;
    }

    return CommandExeStatus::CORRECT;
}


CommandExeStatus FindStation::Run() {
    std::string search_str;
    std::cin >> search_str;

    if (search_str == kAllValue) {
        search_str = "";        
    }

    auto&& list = cli_.FindStation(search_str);

    if (!output_manager_.PointsJsonOutput(cli_, list, "station name", "station id")) {
        output_manager_.GetStreamRef() << "Can not find station by {" << search_str << "} request" << "\n"
            << "try to rescan points" << std::endl;
    }

    return CommandExeStatus::CORRECT;
}


} // namespace commands

} // namespace waybuilder