#ifndef _YA_RASP_JSON_PTR_HPP_
#define _YA_RASP_JSON_PTR_HPP_

#include <nlohmann/json.hpp>

namespace waybuilder {

class YaRaspCli;

struct YaRaspJsonPtr {
    friend class YaRaspCli;

 public:
    static const nlohmann::json::json_pointer kPointId;
    static const nlohmann::json::json_pointer kPointName;
    
    static const nlohmann::json::json_pointer kStationType;

 public:
    static const nlohmann::json::json_pointer kResultCount;
    static const nlohmann::json::json_pointer kRequestFromPointName;
    static const nlohmann::json::json_pointer kRequestToPointName;
    static const nlohmann::json::json_pointer kRequestDate;
    
 public:
    static const nlohmann::json::json_pointer kIntervalFlights;
    static const nlohmann::json::json_pointer kIntervalDepartureDate;

 public:
    static const nlohmann::json::json_pointer kScheduleFlights;
    static const nlohmann::json::json_pointer kScheduleFlightName;
    static const nlohmann::json::json_pointer kScheduleFlightId;
    static const nlohmann::json::json_pointer kVehicleType;
    static const nlohmann::json::json_pointer kVehicleName;
    static const nlohmann::json::json_pointer kScheduleDepartureDate;
    static const nlohmann::json::json_pointer kScheduleArrivalDate;

 private:
    static const nlohmann::json::json_pointer kApiKey;
    static const nlohmann::json::json_pointer kPointListPath;
    static const nlohmann::json::json_pointer kApiUrl;
    static const nlohmann::json::json_pointer kApiVersion;
    static const nlohmann::json::json_pointer kApiLang;

 private:
    static const nlohmann::json::json_pointer kCountry;
    static const nlohmann::json::json_pointer kRegion;
    static const nlohmann::json::json_pointer kCity;
    static const nlohmann::json::json_pointer kStation;
};

} // namespace waybuilder

#endif // _YA_RASP_JSON_PTR_HPP_