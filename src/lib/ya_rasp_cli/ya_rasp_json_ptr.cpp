#include "ya_rasp_json_ptr.hpp"

namespace waybuilder {

const nlohmann::json::json_pointer YaRaspJsonPtr::kApiKey{"/api_key"};
const nlohmann::json::json_pointer YaRaspJsonPtr::kPointListPath{"/point_list_path"};
const nlohmann::json::json_pointer YaRaspJsonPtr::kApiUrl{"/api_url"};
const nlohmann::json::json_pointer YaRaspJsonPtr::kApiVersion{"/api_version"};
const nlohmann::json::json_pointer YaRaspJsonPtr::kApiLang{"/api_lang"};

const nlohmann::json::json_pointer YaRaspJsonPtr::kCountry{"/countries"};
const nlohmann::json::json_pointer YaRaspJsonPtr::kRegion{"/regions"};
const nlohmann::json::json_pointer YaRaspJsonPtr::kCity{"/settlements"};
const nlohmann::json::json_pointer YaRaspJsonPtr::kStation{"/stations"};

const nlohmann::json::json_pointer YaRaspJsonPtr::kPointId{"/codes/yandex_code"};
const nlohmann::json::json_pointer YaRaspJsonPtr::kPointName{"/title"};

const nlohmann::json::json_pointer YaRaspJsonPtr::kStationType{"/station_type"};

const nlohmann::json::json_pointer YaRaspJsonPtr::kResultCount{"/pagination/total"}; 
const nlohmann::json::json_pointer YaRaspJsonPtr::kRequestFromPointName{"/search/from/popular_title"};
const nlohmann::json::json_pointer YaRaspJsonPtr::kRequestToPointName{"/search/to/popular_title"};
const nlohmann::json::json_pointer YaRaspJsonPtr::kRequestDate{"/search/date"};

const nlohmann::json::json_pointer YaRaspJsonPtr::kIntervalFlights{"/interval_segments"};
const nlohmann::json::json_pointer YaRaspJsonPtr::kIntervalDepartureDate{"/start_date"};

const nlohmann::json::json_pointer YaRaspJsonPtr::kScheduleFlights{"/segments"};

const nlohmann::json::json_pointer YaRaspJsonPtr::kScheduleFlightName{"/title"};
const nlohmann::json::json_pointer YaRaspJsonPtr::kScheduleFlightId{"/number"};
const nlohmann::json::json_pointer YaRaspJsonPtr::kVehicleType{"/transport_type"};
const nlohmann::json::json_pointer YaRaspJsonPtr::kVehicleName{"/vehicle"};
const nlohmann::json::json_pointer YaRaspJsonPtr::kScheduleDepartureDate{"/arrival"};
const nlohmann::json::json_pointer YaRaspJsonPtr::kScheduleArrivalDate{"/departure"};

} // namespace waybuilder