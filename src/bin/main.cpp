#include <iostream>

#include <console_cli_app.hpp>

#include <ya_rasp_cli.hpp>

// #include <nlohmann/json.hpp>
// #include <cpr/cpr.h>

int main(int, char**) {
    std::cout << "labwork6" << std::endl;

    // waybuilder::YaRaspCli ya_cli{};
    waybuilder::ConsoleWayBuilderApp app("/home/saintson/my_dir/itmo/labworks/cpp_laba6/labwork6-saintson1/res/api_cfg.json");

    auto ret_code = app.Run();
    return ret_code;
}
