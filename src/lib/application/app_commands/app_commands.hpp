#ifndef _APP_COMMANDS_HPP_
#define _APP_COMMANDS_HPP_

#include <string_view>
#include <iostream>
#include <concepts>
#include <memory>

#include <command_module.hpp>
#include <ya_rasp_cli.hpp>

namespace waybuilder {

using CommandExeStatus = ::commands::CommandExeStatus;

namespace commands {

class Help : public ::commands::CommandBase {
 public:
    CommandExeStatus Run() override { 
        std::cout << kHelpText << std::endl;
        return CommandExeStatus::CORRECT;
    };
 private:
    static std::string_view kHelpText;
};


class Quit : public ::commands::CommandBase {
 public:
    CommandExeStatus Run() override { return CommandExeStatus::EXIT; }
};


class YaRaspApiProjection : public ::commands::CommandBase {
 public:
    YaRaspApiProjection(YaRaspCli& cli) : cli_(cli) {};

 protected:
    YaRaspCli& cli_;
};


class Save : public YaRaspApiProjection {
 public:
    using YaRaspApiProjection::YaRaspApiProjection;
 public:
    CommandExeStatus Run() override;
};


template<std::derived_from<waybuilder::commands::YaRaspApiProjection> YaRaspCommand>
class YaRaspCommandCreator : public ::commands::CommandCreatorBase {
 public:
    YaRaspCommandCreator(YaRaspCli& cli) : cli_{cli} {};
 public:
    std::shared_ptr<::commands::CommandBase> Create() override {
        return std::make_shared<YaRaspCommand>(cli_);
    };
 private:
    YaRaspCli& cli_;
};

} // namespace commands

} // namespace waybuilder


#endif // _APP_COMMANDS_HPP_