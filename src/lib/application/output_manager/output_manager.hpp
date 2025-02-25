#ifndef _OUTPUT_MANAGER_
#define _OUTPUT_MANAGER_

#include <ostream>
#include <iostream>

#include <nlohmann/json.hpp>

#include <ya_rasp_cli.hpp>

namespace waybuilder {


class YaRaspOutputManager {
 public:
    YaRaspOutputManager();
    YaRaspOutputManager(std::ostream& output_stream);

 public:
    bool PointsJsonOutput(YaRaspCli& cli, const nlohmann::json& points_json,
        const std::string& name_colom, const std::string& id_colom);

    void ShedueFlightOutput(const nlohmann::json& flight_json);

    bool WaysJsonOutput(YaRaspCli& cli, const nlohmann::json& ways_json);

 public:
    operator std::ostream&() { return output_stream_; };
    std::ostream& GetStreamRef() { return output_stream_; };
    const std::ostream& GetStreamRef() const { return output_stream_; };

 private:
    std::ostream& output_stream_;
};

} // namespace waybuilder

#endif // _OUTPUT_MANAGER_