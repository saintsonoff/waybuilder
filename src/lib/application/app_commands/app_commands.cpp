#include "app_commands.hpp"


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
        std::cout << "Save is done" << std::endl;
    } else {
        std::cout << "Cannot save config or points, check path" << std::endl;
    }

    return CommandExeStatus::CORRECT;
}


CommandExeStatus ScanPoints::Run() {
    auto resp = cli_.ScanPoints();

    if (resp.status_code == 200) {
        std::cout << "Scan is done" << std::endl;
    } else {
        std::cout << "Cannot scan points, check api config or log journal" << std::endl;

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

    if (!cli_.PointsJsonOutput(std::cout, country_list->get(), "country name", "coutry id")) {
        std::cout << "Get list error" << "\n"
            << "try to rescan points" << std::endl;
    }

    return CommandExeStatus::CORRECT;
}


CommandExeStatus ListRegion::Run() {
    std::string country_id;
    std::cin >> country_id;

    if (country_id == "ALL") {
        
    } else {
        
    }

    return CommandExeStatus::CORRECT;
}


CommandExeStatus ListCity::Run() {

    return CommandExeStatus::CORRECT;
}


CommandExeStatus ListStation::Run() {

    return CommandExeStatus::CORRECT;
}


CommandExeStatus FindCountry::Run() {
    std::string search_str;
    std::cin >> search_str;

    auto&& country_list = cli_.FindCountry(search_str);

    if (!cli_.PointsJsonOutput(std::cout, country_list, "country name", "coutry id")) {
        std::cout << "Can not find country by {" << search_str << "} request" << "\n"
            << "try to rescan points" << std::endl;
    }

    return CommandExeStatus::CORRECT;
}


CommandExeStatus FindRegion::Run() {
    std::string country_id;
    std::cin >> country_id;

    if (country_id == "ALL") {

    } else {
        
    }

    return CommandExeStatus::CORRECT;
}


CommandExeStatus FindCity::Run() {

    return CommandExeStatus::CORRECT;
}


CommandExeStatus FindStation::Run() {

    return CommandExeStatus::CORRECT;
}


} // namespace commands

} // namespace waybuilder