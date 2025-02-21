#ifndef _CONSOLE_CLI_APP_HPP_
#define _CONSOLE_CLI_APP_HPP_

#include "application.hpp"

#include <command_fab.hpp>
#include <ya_rasp_cli.hpp>
#include <output_manager.hpp>

namespace waybuilder {

namespace __detail {
    
template<>
class Application<ApplicationCategories::CONSOLE_CLI> {
 public:
    Application(std::string api_key, std::string point_list_path, std::string api_cfg_path);
    Application(std::string api_cfg_path);

 public:
    ExitStatus Run();

 private:
    void CommandRegistrate();

 private:
    ::commands::CommandFabric commands_;
    YaRaspCli cli_;
    YaRaspOutputManager output_manager_;
};

} // namespace __detail

using ConsoleWayBuilderApp = __detail::Application<ApplicationCategories::CONSOLE_CLI>;

} // namespace waybuilder

#endif // _CONSOLE_CLI_APP_HPP_
