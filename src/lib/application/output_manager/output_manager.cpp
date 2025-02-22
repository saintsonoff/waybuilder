#include "output_manager.hpp"

#include <iostream>

#include <boost/log/trivial.hpp>
#include <boost/log/sources/logger.hpp>

#include <nlohmann/detail/json_pointer.hpp>

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

        if (ways_json.at(YaRaspJsonPtr::kResultCount) == 0) 
            return false;
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
    if (ways_json.contains(YaRaspJsonPtr::kIntervalFlights) && ways_json.at(YaRaspJsonPtr::kIntervalFlights).size()) {
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
    if (ways_json.contains(YaRaspJsonPtr::kScheduleFlights) && ways_json.at(YaRaspJsonPtr::kScheduleFlights).size()) {
        output_stream_ << "Schedule flights list: " << std::endl;
        for (auto& flight : ways_json.at(YaRaspJsonPtr::kScheduleFlights)) {
            try {                    
                output_stream_ << "[" <<  flight_number << "]" << "\n";
                ShedueFlightOutput(flight);
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


void YaRaspOutputManager::ShedueFlightOutput(const nlohmann::json& flight_json) {
    constexpr size_t kInfoIdent = 4;

    const static nlohmann::json::json_pointer kFromJsonPtr{"/from"};
    const static nlohmann::json::json_pointer kToJsonPtr{"/to"};
    const static nlohmann::json::json_pointer kHasTransfers{"/has_transfers"};
    const static nlohmann::json::json_pointer kTransfers{"/details"};
    const static nlohmann::json::json_pointer kDepartureFrom{"/departure_from"};
    const static nlohmann::json::json_pointer kArrivalTo{"/arrival_to"};
    const static nlohmann::json::json_pointer kIsTransfer{"/is_transfer"};
    const static nlohmann::json::json_pointer kTransferPoint{"/transfer_point"};

    const static nlohmann::json::json_pointer kNonTransferFlightInfoPrefix{"/thread"};

    if (flight_json.contains(kHasTransfers) && flight_json.at(kHasTransfers)) {
        for (auto&& transfer : flight_json.at(kTransfers)) {
            if (transfer.contains(kIsTransfer) && transfer.at(kIsTransfer)) {
                output_stream_
                    << std::setw(kInfoIdent) << std::setw(kInfoIdent) << "transfer point ==> " << transfer.at(kTransferPoint / YaRaspJsonPtr::kPointName).get_ref<const std::string&>() << "\n";
            } else {
                output_stream_
                    << std::setw(kInfoIdent) << transfer.at(kNonTransferFlightInfoPrefix / YaRaspJsonPtr::kScheduleFlightName).get_ref<const std::string&>() << "\n"
                    << std::setw(kInfoIdent) << "flight name: " << transfer.at(kNonTransferFlightInfoPrefix / YaRaspJsonPtr::kScheduleFlightId).get_ref<const std::string&>() << "\n"
                    << std::setw(kInfoIdent) << "transport type: " << transfer.at(kNonTransferFlightInfoPrefix / YaRaspJsonPtr::kVehicleType).get_ref<const std::string&>() << "\n"
                    << (!transfer.at(kNonTransferFlightInfoPrefix / YaRaspJsonPtr::kVehicleName).is_null() ? ("trasport model: " + transfer.at(kNonTransferFlightInfoPrefix / YaRaspJsonPtr::kVehicleName).get_ref<const std::string&>()) + "\n" : "")
                    << std::setw(kInfoIdent) << "departure date: " << transfer.at(YaRaspJsonPtr::kScheduleDepartureDate).get_ref<const std::string&>() << "\n"
                    << std::setw(kInfoIdent) << "arrival date: " << transfer.at(YaRaspJsonPtr::kScheduleArrivalDate).get_ref<const std::string&>() << "\n"

                    << std::setw(kInfoIdent) << "departure point: " << transfer.at(kFromJsonPtr / YaRaspJsonPtr::kPointName).get_ref<const std::string&>() << "\n" 
                    << std::setw(kInfoIdent) << std::setw(kInfoIdent)
                    << std::setw(kInfoIdent) << "arrival point: " << transfer.at(kToJsonPtr / YaRaspJsonPtr::kPointName).get_ref<const std::string&>() << "\n" 
                    << std::endl;
            }
        }
    } else {
        output_stream_
            << std::setw(kInfoIdent) << flight_json.at(kNonTransferFlightInfoPrefix / YaRaspJsonPtr::kScheduleFlightName).get_ref<const std::string&>() << "\n"
            << std::setw(kInfoIdent) << "flight name: " << flight_json.at(kNonTransferFlightInfoPrefix / YaRaspJsonPtr::kScheduleFlightId).get_ref<const std::string&>() << "\n"
            << std::setw(kInfoIdent) << "transport type: " << flight_json.at(kNonTransferFlightInfoPrefix / YaRaspJsonPtr::kVehicleType).get_ref<const std::string&>() << "\n"
            << (!flight_json.at(kNonTransferFlightInfoPrefix / YaRaspJsonPtr::kVehicleName).is_null() ? ("trasport model: " + flight_json.at(kNonTransferFlightInfoPrefix / YaRaspJsonPtr::kVehicleName).get_ref<const std::string&>()) + "\n" : "")
            << std::setw(kInfoIdent) << "departure date: " << flight_json.at(YaRaspJsonPtr::kScheduleDepartureDate).get_ref<const std::string&>() << "\n"
            << std::setw(kInfoIdent) << "arrival date: " << flight_json.at(YaRaspJsonPtr::kScheduleArrivalDate).get_ref<const std::string&>() << "\n"

            << std::setw(kInfoIdent) << "departure point: " << flight_json.at(kFromJsonPtr / YaRaspJsonPtr::kPointName).get_ref<const std::string&>() << "\n" 
            << std::setw(kInfoIdent) << std::setw(kInfoIdent)
                << "departure station type: " << flight_json.at(kFromJsonPtr / YaRaspJsonPtr::kStationType).get_ref<const std::string&>() << "\n" 
            << std::setw(kInfoIdent) << "arrival point: " << flight_json.at(kToJsonPtr / YaRaspJsonPtr::kPointName).get_ref<const std::string&>() << "\n" 
            << std::setw(kInfoIdent) << std::setw(kInfoIdent)
                << "arrival station type: " << flight_json.at(kToJsonPtr / YaRaspJsonPtr::kStationType).get_ref<const std::string&>() << "\n" 
            << std::endl;
    }
}

} // namespace waybuilder