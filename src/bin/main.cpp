#include <iostream>

#include <console_cli_app.hpp>

#include <ya_rasp_cli.hpp>

// #include <nlohmann/json.hpp>
// #include <cpr/cpr.h>

int main(int, char**) {
    std::cout << "labwork6" << std::endl;

    waybuilder::YaRaspCli ya_cli{"./"};
    // waybuilder::ConsoleWayBuilderApp app;

    // auto ret_code = app.Run();
    // return ret_code;
}
