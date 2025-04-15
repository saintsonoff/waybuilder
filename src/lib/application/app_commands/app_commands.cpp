#include "app_commands.hpp"

#include <cstddef>
#include <iomanip>
#include <sstream>
#include <string>
#include <memory>
#include <iostream>
#include <chrono>
#include <ctime>

#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/trivial.hpp>

#include <nlohmann/json.hpp>

namespace waybuilder {

namespace commands {

std::string Help::kHelpText =
R"help(
* help 
    - this message
* save
    - save config

* change lang [lang] "lang in code by  ISO 639 & ISO 3166 | in format xx_XX"
    - change language of response

* list ways [from_id] [to_id] [date] "date in format [xxxx-xx-xx, today, tomorrow]"
    - get list of avalible ways
* list country
    - get list of avalible coutnry
* list region [country_id]
    - get list of avalible regions in current coutnry
* list city [country_id] [region_id]
    - get list of avalible cities in current region of current coutnry 
* list station [country_id] [region_id] [city_id]
    - get list of avalible stations in current region of current coutnry of current city 
* find country [find_substring]
    - get list of avalible coutry by similar request 
* find region [find_substring]
    - get list of avalible region by similar request 
* find city [find_substring]
    - get list of avalible cities by similar request 
* find station [find_substring]
    - get list of avalible stations by similar request

* logdir
    - path to directory to log journal
)help";


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
    if (cli_.ScanPoints().status_code == 200) {
        output_manager_.GetStreamRef() << "Change language is done" << std::endl;
    } else {
        output_manager_.GetStreamRef() << "Unknowing lang, check yandex langs table" << "\n"
            << "Actual language: " << old_lang << std::endl;
        cli_.SetLang(old_lang);
    }

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