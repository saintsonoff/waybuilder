#include "output_manager.hpp"
#include "nlohmann/detail/json_pointer.hpp"

#include <iostream>

#include <boost/log/trivial.hpp>
#include <boost/log/sources/logger.hpp>

#include <ya_rasp_cli.hpp>
#include <ya_rasp_json_ptr.hpp>

namespace waybuilder {

YaRaspOutputManager::YaRaspOutputManager() : output_stream_(std::cout) {};


YaRaspOutputManager::YaRaspOutputManager(std::ostream& output_stream) : output_stream_(output_stream) {};


bool YaRaspOutputManager::PointsJsonOutput(YaRaspCli& cli, const nlohmann::json& points_json,
    const std::string& name_colom, const std::string& id_colom) {
    static constexpr size_t kCollomSpaceOffset = 20;

    if (points_json.empty()) {
        BOOST_LOG_SEV(cli.GetLoggerRef(), boost::log::trivial::error) 
            << "output point error, list of points is empty"; 
        return false;
    }

    output_stream_ << name_colom << " " << std::setw(kCollomSpaceOffset) << id_colom << std::endl;

    try {
        for (auto&& point : points_json) {
            if (point.contains(YaRaspJsonPtr::kPointName) && point.contains(YaRaspJsonPtr::kPointId)) {
                output_stream_
                    << point.at(YaRaspJsonPtr::kPointName).get<std::string>() << std::setw(kCollomSpaceOffset)
                    << point.at(YaRaspJsonPtr::kPointId).get<std::string>() << std::endl;
            }
        }
    } catch (nlohmann::json::out_of_range& ex) {
        BOOST_LOG_SEV(cli.GetLoggerRef(), boost::log::trivial::error) 
            << "output point error" << " | " 
            << "exception id: " << ex.id << " | "
            << ex.what();
        return false;
    }
    return true;
};
  

bool YaRaspOutputManager::WaysJsonOutput(YaRaspCli& cli, const nlohmann::json& ways_json) {
    // title output
    if (ways_json.contains(YaRaspJsonPtr::kResultCount) && ways_json.contains(YaRaspJsonPtr::kRequestFromPointName)
        && ways_json.contains(YaRaspJsonPtr::kRequestToPointName) && ways_json.contains(YaRaspJsonPtr::kRequestDate)) {
        output_stream_ << "Ways list" << "\n"
            << "from: " << ways_json.at(YaRaspJsonPtr::kRequestFromPointName) << "\n"
            << "to: " << ways_json.at(YaRaspJsonPtr::kRequestToPointName) << "\n"
            << "date: " << ways_json.at(YaRaspJsonPtr::kRequestDate) << std::endl;

        output_stream_ << "Result count: " << ways_json.at(YaRaspJsonPtr::kResultCount) << std::endl;

        output_stream_ << "\n";
    } else {
        BOOST_LOG_SEV(cli.GetLoggerRef(), boost::log::trivial::error) 
            << "output ways error" << " | "
            << "ways json is not correspond to format";
        output_stream_ << "output ways error: "
            << "check log journal" << std::endl;
        return false;
    }


    size_t flight_number = 0;
    constexpr size_t kInfoIdent = 4;
    const static nlohmann::json::json_pointer kFromJsonPtr{"/from"};
    const static nlohmann::json::json_pointer kToJsonPtr{"/to"};


    // Interval flights
    if (ways_json.contains(YaRaspJsonPtr::kIntervalFlights)) {
        output_stream_ << "Interval flights list: " << std::endl;
        for (auto& flight : ways_json.at(YaRaspJsonPtr::kIntervalFlights)) {
            try {                    
                output_stream_ << "[" <<  flight_number << "]" << "\n";
                output_stream_ 
                << std::setw(kInfoIdent) << "departure date: " << flight.at(YaRaspJsonPtr::kScheduleDepartureDate) << "\n" 
                    << std::setw(kInfoIdent) << "departure point: " << flight.at(kFromJsonPtr / YaRaspJsonPtr::kPointName) << "\n" 
                    << std::setw(kInfoIdent) << std::setw(kInfoIdent) << "departure station type: " << flight.at(kFromJsonPtr / YaRaspJsonPtr::kStationType) << "\n" 
                    << std::setw(kInfoIdent) << "arrival point: " << flight.at(kToJsonPtr / YaRaspJsonPtr::kPointName) << "\n" 
                    << std::setw(kInfoIdent) << std::setw(kInfoIdent) << "arrival station type: " << flight.at(kToJsonPtr / YaRaspJsonPtr::kStationType) << "\n" 
                    << std::endl;
            } catch (nlohmann::json::out_of_range& ex) {
                BOOST_LOG_SEV(cli.GetLoggerRef(), boost::log::trivial::error) 
                    << "output ways error" << " | " 
                    << "exception id: " << ex.id << " | "
                    << ex.what();
            }
            ++flight_number;
        }
        output_stream_ << "\n";
    }

    // Schedule flights
    if (ways_json.contains(YaRaspJsonPtr::kScheduleFlights)) {
        output_stream_ << "Schedule flights list: " << std::endl;
        for (auto& flight : ways_json.at(YaRaspJsonPtr::kScheduleFlights)) {
            try {                    
                output_stream_ << "[" <<  flight_number << "]" << "\n";
                output_stream_
                    << std::setw(kInfoIdent) << flight.at(YaRaspJsonPtr::kScheduleFlightName).get_ref<const std::string&>() << "\n"
                    << std::setw(kInfoIdent) << "flight name: " << flight.at(YaRaspJsonPtr::kScheduleFlightId).get_ref<const std::string&>() << "\n"
                    << std::setw(kInfoIdent) << "transport type: " << flight.at(YaRaspJsonPtr::kVehicleType).get_ref<const std::string&>() << "\n"
                    << std::setw(kInfoIdent)
                        << (!flight.at(YaRaspJsonPtr::kVehicleName).is_null() ? ("trasport model: " + flight.at(YaRaspJsonPtr::kVehicleName).get_ref<const std::string&>()) + "\n" : "")
                    << std::setw(kInfoIdent) << "departure date: " << flight.at(YaRaspJsonPtr::kScheduleDepartureDate).get_ref<const std::string&>() << "\n"
                    << std::setw(kInfoIdent) << "arrival date: " << flight.at(YaRaspJsonPtr::kScheduleArrivalDate).get_ref<const std::string&>() << "\n"

                    << std::setw(kInfoIdent) << "departure point: " << flight.at(kFromJsonPtr / YaRaspJsonPtr::kPointName).get_ref<const std::string&>() << "\n" 
                    << std::setw(kInfoIdent) << std::setw(kInfoIdent)
                        << "departure station type: " << flight.at(kFromJsonPtr / YaRaspJsonPtr::kStationType).get_ref<const std::string&>() << "\n" 
                    << std::setw(kInfoIdent) << "arrival point: " << flight.at(kToJsonPtr / YaRaspJsonPtr::kPointName).get_ref<const std::string&>() << "\n" 
                    << std::setw(kInfoIdent) << std::setw(kInfoIdent)
                        << "arrival station type: " << flight.at(kToJsonPtr / YaRaspJsonPtr::kStationType).get_ref<const std::string&>() << "\n" 
                    << std::endl;
            } catch (nlohmann::json::out_of_range& ex) {
                BOOST_LOG_SEV(cli.GetLoggerRef(), boost::log::trivial::error) 
                    << "output ways error" << " | " 
                    << "exception id: " << ex.id << " | "
                    << ex.what();
            }
            ++flight_number;
        }
        output_stream_ << "\n";
    }


    return true;
}


} // namespace waybuilder