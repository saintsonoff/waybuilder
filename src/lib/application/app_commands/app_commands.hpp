#ifndef _APP_COMMANDS_HPP_
#define _APP_COMMANDS_HPP_

#include <sstream>
#include <string_view>
#include <iostream>
#include <concepts>
#include <memory>

#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/trivial.hpp>

#include <command_module.hpp>
#include <ya_rasp_cli.hpp>
#include <output_manager.hpp>
#include <lru_cache.hpp>

namespace waybuilder {

using CommandExeStatus = ::commands::CommandExeStatus;

namespace commands {

const std::string kAllValue = "all";

class YaRaspApiProjection : public ::commands::CommandBase {
 public:
    YaRaspApiProjection(YaRaspCli& cli, YaRaspOutputManager& output_manager)
        : cli_(cli), output_manager_(output_manager) {};

 protected:
    YaRaspCli& cli_;
    YaRaspOutputManager& output_manager_;
};
   

template<std::derived_from<waybuilder::commands::YaRaspApiProjection> YaRaspCommand>
class YaRaspCommandCreator : public ::commands::CommandCreatorBase {
 public:
    YaRaspCommandCreator(YaRaspCli& cli, YaRaspOutputManager& output_manager)
        : cli_{cli}, output_manager_(output_manager) {  };
 public:
    std::shared_ptr<::commands::CommandBase> Create() override {
        return std::make_shared<YaRaspCommand>(cli_, output_manager_);
    };
 private:
    YaRaspCli& cli_;
    YaRaspOutputManager& output_manager_;
};


class Help : public YaRaspApiProjection {
 public:
    using YaRaspApiProjection::YaRaspApiProjection;

 public:
    CommandExeStatus Run() override { 
        output_manager_.GetStreamRef() << kHelpText << std::endl;
        return CommandExeStatus::CORRECT;
    };
 private:
    static std::string kHelpText;
};


class Logdir : public YaRaspApiProjection {
 public:
    using YaRaspApiProjection::YaRaspApiProjection;

 public:
    CommandExeStatus Run() override { 
        output_manager_.GetStreamRef() << cli_.GetLoggerPath() << std::endl;
        return CommandExeStatus::CORRECT;
    };
 private:
    static std::string kHelpText;
};


class Quit : public ::commands::CommandBase {
 public:
    CommandExeStatus Run() override { return CommandExeStatus::EXIT; }
};


class Save : public YaRaspApiProjection {
 public:
    using YaRaspApiProjection::YaRaspApiProjection;
 public:
    CommandExeStatus Run() override;
};


class ChangeBase : public YaRaspApiProjection {
 public:
    using YaRaspApiProjection::YaRaspApiProjection;
};


class ChangeLang : public ChangeBase {
 public:
    using ChangeBase::ChangeBase;
 public:
    CommandExeStatus Run() override;
};


template<>
class YaRaspCommandCreator<ChangeBase> : public ::commands::CommandCreatorBase {
 public:
    YaRaspCommandCreator(YaRaspCli& cli, YaRaspOutputManager& output_manager)
        : cli_{cli}, output_manager_{output_manager} {  };
 public:
    std::shared_ptr<::commands::CommandBase> Create() override {
        std::string change_type;
        std::cin >> change_type;
        if (change_type == "lang") {
            return std::make_shared<ChangeLang>(cli_, output_manager_);
        } else {
            return std::make_shared<::commands::InvalidCommand>();
        }
    };
 private:
    YaRaspCli& cli_;
    YaRaspOutputManager& output_manager_;
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
    YaRaspCommandCreator(YaRaspCli& cli, YaRaspOutputManager& output_manager)
        : cli_{cli}, output_manager_{output_manager} {  };
 public:
    std::shared_ptr<::commands::CommandBase> Create() override {
        std::string scan_type;
        std::cin >> scan_type;
        if (scan_type == "points") {
            return std::make_shared<ScanPoints>(cli_, output_manager_);
        } else {
            return std::make_shared<::commands::InvalidCommand>();
        }
    };
 private:
    YaRaspCli& cli_;
    YaRaspOutputManager& output_manager_;
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


template<typename CacherType>
class ListWay : public ListBase {
 public:
    ListWay(YaRaspCli& cli, YaRaspOutputManager& output_manager,
        CacherType& cache) : ListBase(cli, output_manager), cache_(cache) {  };
 public:
    CommandExeStatus Run() override;
 private:
    std::stringstream GetCurrentTime(int year_offset = 0, int month_offset = 0, int day_offset = 0,
        int hour_offset = 0, int minut_offset = 0, int second_offset = 0);
 private:
    CacherType& cache_;
};


template<typename CacherType>
std::stringstream ListWay<CacherType>::GetCurrentTime(int year_offset, int month_offset, int day_offset,
    int hour_offset, int minut_offset, int second_offset) {
    auto current_timepoint = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(current_timepoint);

    std::tm* current_time_ptr = std::localtime(&now_time_t);
    std::stringstream time_conversion_stream;

    current_time_ptr->tm_year += year_offset;
    current_time_ptr->tm_mon += month_offset;
    current_time_ptr->tm_mday += day_offset;

    current_time_ptr->tm_hour += hour_offset;
    current_time_ptr->tm_min += minut_offset;
    current_time_ptr->tm_sec += second_offset;

    time_conversion_stream << std::put_time(current_time_ptr, "%Y-%m-%d %H:%M");
    return time_conversion_stream;
}


template<typename CacherType>
CommandExeStatus ListWay<CacherType>::Run() {
    std::string from_point_id;
    std::string to_point_id;
    std::string date;

    std::cin >> from_point_id >> to_point_id >> date;


    if (date == "today") {
        GetCurrentTime() >> date;
    } else if (date == "tomorrow") {
        GetCurrentTime(0, 0, 1) >> date;
    }

    nlohmann::json ways_json;

    auto ways_opt = cache_.get(from_point_id + to_point_id + date);
     
    if (ways_opt) {
        output_manager_.GetStreamRef() << "from_cache\n";
        ways_json = ways_opt.value();
    } else {
        auto resp = cli_.ScanWays(from_point_id, to_point_id, date, true);

        if (resp.status_code != 200) {
            output_manager_.GetStreamRef() << "Ways scan error, check log journal" << std::endl;
            return CommandExeStatus::CORRECT;
        }

        try {
            ways_json = nlohmann::json::parse(resp.text);
        } catch (nlohmann::json::parse_error& ex) {
            BOOST_LOG_SEV(cli_.GetLoggerRef(), boost::log::trivial::error)
                << "parse ways json error " << " | "
                << "id: " << ex.id << " | "
                << "text: " << ex.what();
            output_manager_.GetStreamRef() << "Ways scan error, check log journal" << std::endl;
            return CommandExeStatus::CORRECT;
        }

    }

    if (!output_manager_.WaysJsonOutput(cli_, ways_json)) {
        output_manager_.GetStreamRef() << "Can not find ways by {"
            << (from_point_id.empty() ? "" : " from point:  " + from_point_id + " / ") 
            << (to_point_id.empty() ? "" : " to point:  " + to_point_id + " / ") 
            << (date.empty() ? "" : " date: " + date) 
            << "} request" << "\n"
            << "try to rescan points" << std::endl;
    } else {

        cache_.insert({from_point_id + to_point_id + date}, ways_json);
    }        

    return CommandExeStatus::CORRECT;
}


template<std::derived_from<ListBase> YaRaspListComand, typename CacherType>
class YaRaspApiListCreator : public ::commands::CommandCreatorBase {
 public:
    YaRaspApiListCreator(YaRaspCli& cli, YaRaspOutputManager& output_manager,
        CacherType& cache)
            : cli_{cli}, output_manager_{output_manager}, cache_{cache} {  };
 public:
    std::shared_ptr<::commands::CommandBase> Create() override {
        std::string list_of;
        std::cin >> list_of;
        if (list_of == "country") {
            return std::make_shared<ListCountry>(cli_, output_manager_);
        } else if (list_of == "region") {
            return std::make_shared<ListRegion>(cli_, output_manager_);
        } else if (list_of == "city") {
            return std::make_shared<ListCity>(cli_, output_manager_);
        } else if (list_of == "station") {
            return std::make_shared<ListStation>(cli_, output_manager_);
        } else if (list_of == "way") {
            return std::make_shared<ListWay<CacherType>>(cli_, output_manager_, cache_);
        } else {
            return std::make_shared<::commands::InvalidCommand>();
        }
    };
 private:
    YaRaspCli& cli_;
    YaRaspOutputManager& output_manager_;
    CacherType& cache_;
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
    YaRaspCommandCreator(YaRaspCli& cli, YaRaspOutputManager& output_manager)
        : cli_{cli}, output_manager_(output_manager) {  };
 public:
    std::shared_ptr<::commands::CommandBase> Create() override {
        std::string list_of;
        std::cin >> list_of;
        if (list_of == "country") {
            return std::make_shared<FindCountry>(cli_, output_manager_);
        } else if (list_of == "region") {
            return std::make_shared<FindRegion>(cli_, output_manager_);
        } else if (list_of == "city") {
            return std::make_shared<FindCity>(cli_, output_manager_);
        } else if (list_of == "station") {
            return std::make_shared<FindStation>(cli_, output_manager_);
        } else {
            return std::make_shared<::commands::InvalidCommand>();
        }
    };
 private:
    YaRaspCli& cli_;
    YaRaspOutputManager& output_manager_;
};


} // namespace commands

} // namespace waybuilder


#endif // _APP_COMMANDS_HPP_