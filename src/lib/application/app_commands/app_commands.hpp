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

const std::string kAllValue = "all";

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


class Save : public YaRaspApiProjection {
 public:
    using YaRaspApiProjection::YaRaspApiProjection;
 public:
    CommandExeStatus Run() override;
};


class ScanBase : public YaRaspApiProjection {
 public:
    using YaRaspApiProjection::YaRaspApiProjection;
};


class ScanPoints : public ScanBase {
 public:
    using ScanBase::ScanBase;
 public:
    CommandExeStatus Run() override;
};


template<>
class YaRaspCommandCreator<ScanBase> : public ::commands::CommandCreatorBase {
 public:
    YaRaspCommandCreator(YaRaspCli& cli) : cli_{cli} {};
 public:
    std::shared_ptr<::commands::CommandBase> Create() override {
        std::string scan_type;
        std::cin >> scan_type;
        if (scan_type == "way") {
            // return std::make_shared<YaRaspCommand>(cli_);
        } else if (scan_type == "points") {
            return std::make_shared<ScanPoints>(cli_);
        } else {
            return std::make_shared<::commands::InvalidCommand>();
        }
    };
 private:
    YaRaspCli& cli_;

};


class ListBase : public YaRaspApiProjection {
 public:
    using YaRaspApiProjection::YaRaspApiProjection;
};


class ListCountry : public ListBase {
 public:
    using ListBase::ListBase;
 public:
    CommandExeStatus Run() override;
};


class ListRegion : public ListBase {
 public:
    using ListBase::ListBase;
 public:
    CommandExeStatus Run() override;
};


class ListCity : public ListBase {
 public:
    using ListBase::ListBase;
 public:
    CommandExeStatus Run() override;
};


class ListStation : public ListBase {
 public:
    using ListBase::ListBase;
 public:
    CommandExeStatus Run() override;
};


template<>
class YaRaspCommandCreator<ListBase> : public ::commands::CommandCreatorBase {
 public:
    YaRaspCommandCreator(YaRaspCli& cli) : cli_{cli} {};
 public:
    std::shared_ptr<::commands::CommandBase> Create() override {
        std::string list_of;
        std::cin >> list_of;
        if (list_of == "country") {
            return std::make_shared<ListCountry>(cli_);
        } else if (list_of == "region") {
            return std::make_shared<ListRegion>(cli_);
        } else if (list_of == "city") {
            return std::make_shared<ListCity>(cli_);
        } else if (list_of == "station") {
            return std::make_shared<ListStation>(cli_);
        } else {
            return std::make_shared<::commands::InvalidCommand>();
        }
    };
 private:
    YaRaspCli& cli_;
};


class FindBase : public YaRaspApiProjection {
 public:
    using YaRaspApiProjection::YaRaspApiProjection;
};


class FindCountry : public FindBase {
 public:
    using FindBase::FindBase;
 public:
    CommandExeStatus Run() override;
};


class FindRegion : public FindBase {
 public:
    using FindBase::FindBase;
 public:
    CommandExeStatus Run() override;
};


class FindCity : public FindBase {
 public:
    using FindBase::FindBase;
 public:
    CommandExeStatus Run() override;
};


class FindStation : public FindBase {
 public:
    using FindBase::FindBase;
 public:
    CommandExeStatus Run() override;
};


template<>
class YaRaspCommandCreator<FindBase> : public ::commands::CommandCreatorBase {
 public:
    YaRaspCommandCreator(YaRaspCli& cli) : cli_{cli} {};
 public:
    std::shared_ptr<::commands::CommandBase> Create() override {
        std::string list_of;
        std::cin >> list_of;
        if (list_of == "country") {
            return std::make_shared<FindCountry>(cli_);
        } else if (list_of == "region") {
            return std::make_shared<FindRegion>(cli_);
        } else if (list_of == "city") {
            return std::make_shared<FindCity>(cli_);
        } else if (list_of == "station") {
            return std::make_shared<FindStation>(cli_);
        } else {
            return std::make_shared<::commands::InvalidCommand>();
        }
    };
 private:
    YaRaspCli& cli_;
};


} // namespace commands

} // namespace waybuilder


#endif // _APP_COMMANDS_HPP_