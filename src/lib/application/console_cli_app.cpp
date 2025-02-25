#include "console_cli_app.hpp"

#include <utility>
#include <string>

#include <command_module.hpp>

#include <app_commands.hpp>
#include <output_manager.hpp>

namespace waybuilder {

namespace __detail {

Application<ApplicationCategories::CONSOLE_CLI>::Application(std::string api_key, std::string point_list_path, std::string api_cfg_path) 
  : cli_{api_key, point_list_path, api_cfg_path, "ru_RU"}, output_manager_{std::cout} {
    CommandRegistrate();
};


Application<ApplicationCategories::CONSOLE_CLI>::Application(std::string api_cfg_path)
    : cli_{api_cfg_path}, output_manager_{std::cout} {
    CommandRegistrate();
}


void Application<ApplicationCategories::CONSOLE_CLI>::CommandRegistrate() {
    commands_.Add(
        std::pair<std::string, ::commands::CommandCreator<commands::Quit>>{"quit", {}},
        std::pair<std::string, commands::YaRaspCommandCreator<commands::Help>>{"help", {cli_, output_manager_}},
        std::pair<std::string, commands::YaRaspCommandCreator<commands::Save>>{"save", {cli_, output_manager_}},
        std::pair<std::string, commands::YaRaspCommandCreator<commands::ChangeBase>>{"change", {cli_, output_manager_}},
        std::pair<std::string, commands::YaRaspCommandCreator<commands::ScanBase>>{"scan", {cli_, output_manager_}},
        std::pair<std::string, commands::YaRaspApiListCreator<commands::ListBase, CacheType>>{"list", {cli_, output_manager_, cache_}},
        std::pair<std::string, commands::YaRaspApiFindCreator<commands::FindBase, CacheType>>{"find", {cli_, output_manager_, cache_}},
        std::pair<std::string, commands::YaRaspCommandCreator<commands::Logdir>>{"logdir", {cli_, output_manager_}}
    );
};

ExitStatus Application<ApplicationCategories::CONSOLE_CLI>::Run() {
    static constexpr std::string prefix_lable = "[waybuilder]>  ";

    while (true) {
        std::cout << prefix_lable;

        auto cmd = commands_.GetCommand(std::cin);
        auto exec_status = cmd->Run();

        if (exec_status == CommandExeStatus::INVALID_INPUT) {
            std::cout << "invalid input, check [help]" << std::endl;
        } else if (exec_status == CommandExeStatus::EXIT) {
            break;
        } else if (exec_status == CommandExeStatus::FAIL) {
            return ExitStatus::FAIL;
        }
    }

    return ExitStatus::CORRECT; 
};


}

}