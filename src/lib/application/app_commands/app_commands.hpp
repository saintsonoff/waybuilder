#ifndef _APP_COMMANDS_HPP_
#define _APP_COMMANDS_HPP_

#include "nlohmann/json_fwd.hpp"
#include "ya_rasp_json_ptr.hpp"
#include <array>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <chrono>
#include <compare>
#include <codecvt>
#include <filesystem>
#include <type_traits>
#include <sstream>
#include <string>
#include <string_view>
#include <iostream>
#include <concepts>
#include <memory>
#include <limits>
#include <utility>
#include <vector>
#include <queue>

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
        output_manager_.GetStreamRef() << std::filesystem::absolute(std::filesystem::path(cli_.GetLoggerPath())) << std::endl;
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
        CacherType& cache) : ListBase(cli, output_manager), cache_(cache) {
        InputParams();
    };

    ListWay(YaRaspCli& cli, YaRaspOutputManager& output_manager, CacherType& cache,
        const std::string& from_point_id, const std::string& to_point_id, const std::string& date)
            : ListBase(cli, output_manager), cache_(cache),
                from_point_id_(from_point_id), to_point_id_(to_point_id), date_(date) {  };

 public:
    CommandExeStatus Run() override;

 private:
    void InputParams();
    void CacheRefresh();

 private:
    CacherType& cache_;

 private:
    std::string from_point_id_;
    std::string to_point_id_;
    std::string date_ = "today";
};


template<typename CacherType>
void ListWay<CacherType>::InputParams() { 
    std::cin >> from_point_id_ >> to_point_id_ >> date_;
};

template<typename CacherType>
void ListWay<CacherType>::CacheRefresh() {
    const std::time_t kWayCacheLifetime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::hours(4)).count();
    std::time_t current_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    std::vector<std::pair<std::time_t, std::string>> key_time_cont;
    key_time_cont.reserve(cache_.size());
    for (auto& way_cache : cache_) {
        key_time_cont.emplace_back(way_cache.second.first.second, way_cache.first);
    }

    for (auto& key_time : key_time_cont) {
        if (current_time - key_time.first > kWayCacheLifetime) {
            cache_.erase(key_time.second);
        }
    }
}


template<typename CacherType>
CommandExeStatus ListWay<CacherType>::Run() {
    std::stringstream ss_time_buff;

    if (date_ == "today") {
        std::time_t current_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        ss_time_buff << std::put_time(std::localtime(&current_time), "%Y-%m-%d");
        ss_time_buff >> date_;
    } else if (date_ == "tomorrow") {
        std::time_t current_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() + std::chrono::days(1));
        ss_time_buff << std::put_time(std::localtime(&current_time), "%Y-%m-%d");
        ss_time_buff >> date_;
    }

    nlohmann::json ways_json;

    CacheRefresh();
    auto ways_opt = cache_.get(from_point_id_ + to_point_id_ + date_);
     
    if (ways_opt) {
        ways_json = ways_opt.value().first;
    } else {
        auto resp = cli_.ScanWays(from_point_id_, to_point_id_, date_, true);

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
            output_manager_.GetStreamRef()
                << "Ways scan error, check log journal" << "\n"
                << resp.text << "\n"
                << std::endl;
            return CommandExeStatus::CORRECT;
        }
    }

    if (!output_manager_.WaysJsonOutput(cli_, ways_json)) {
        output_manager_.GetStreamRef() << "Can not find ways by {"
            << (from_point_id_.empty() ? "" : " from point:  " + from_point_id_ + " / ") 
            << (to_point_id_.empty() ? "" : " to point:  " + to_point_id_ + " / ") 
            << (date_.empty() ? "" : " date: " + date_) 
            << "} request" << "\n"
            << "try to rescan points" << std::endl;
    } else {
        cache_.insert(
            {from_point_id_ + to_point_id_ + date_},
            {ways_json, std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())}
        );
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


template<typename CacherType>
class FindWay : public ListBase {
 private:
    class DynamicContType {
     public:
        using cont_type = std::array<std::vector<size_t>, 3>;

     private:
        class SubContType {
         public:
            using value_t = size_t;

         public:
            SubContType(std::vector<value_t>& subcont) : subcont_(subcont) {  };

         public:
            std::vector<value_t>::reference operator[](int64_t index) {
                return (index < 0) ? subcont_[(index % size() + size()) % size()] : subcont_[index]; 
            };

            std::vector<value_t>::const_reference operator[](int64_t index) const {
                return (index < 0) ? subcont_[(index % size() + size()) % size()] : subcont_[index]; 
            };

         public:
            std::vector<value_t>::iterator begin() { return subcont_.begin(); };
            std::vector<value_t>::const_iterator cbegin() const { return subcont_.cbegin(); };
            std::vector<value_t>::iterator end() { return subcont_.end(); };
            std::vector<value_t>::const_iterator cend() const { return subcont_.cend(); };

            size_t size() const { return subcont_.size(); };

         private:
            std::vector<value_t>& subcont_;
        };

        class ConstSubContType {
         public:
            using value_t = size_t;

         public:
            ConstSubContType(const std::vector<value_t>& subcont) : subcont_(subcont) {  };

         public:
            std::vector<value_t>::const_reference operator[](int64_t index) const {
                return (index < 0) ? subcont_[(index % size() + size()) % size()] : subcont_[index];
            };

         public:
            std::vector<value_t>::const_iterator cbegin() const { return subcont_.cbegin(); };
            std::vector<value_t>::const_iterator cend() const { return subcont_.cend(); };

            size_t size() const { return subcont_.size(); };

         private:
            const std::vector<value_t>& subcont_;
        };

     public:
        DynamicContType(size_t size) : cont_{
                std::vector<size_t>(size, 0),
                std::vector<size_t>(size, 0),
                std::vector<size_t>(size, 0)
            } {  };

     public:
        SubContType operator[](int64_t index) {
            if (index < 0) {
                return {cont_[((index) % 3 + 3) % 3]};
            } else {
                return {cont_[index % 3]}; 
            }
        };

        ConstSubContType operator[](int64_t index) const {
            if (index < 0) {
                return {cont_[((index) % size() + size()) % size()]};
            } else {
                return {cont_[index % size()]}; 
            }
        };

     private:
        size_t size() const { return cont_.size(); };

     private:
        std::array<std::vector<size_t>, 3> cont_;
    };

    template<size_t kMaxSize>
    class FixedPriorityQueue {
     private:
        using value_t = std::pair<size_t, nlohmann::json>;
        static bool value_t_comparator(const value_t& lhs, const value_t& rhs) { return lhs.first < rhs.first; };
     public:
        size_t size() const { return cont_.size(); };

        void push(const value_t& value) {
            if (cont_.size() == kMaxSize) {
                pop();
            }

            cont_.push(value);
        };

        template<typename... ArgsT>
        void emplace(ArgsT&&... args) {
            if (cont_.size() == kMaxSize) {
                pop();
            }

            cont_.emplace(std::forward<ArgsT>(args)...);
        };

        const value_t& top() { return cont_.top(); };

        void pop() {
            cont_.pop();
        };

     private:
        std::priority_queue<
            value_t,
            std::vector<value_t>,
            decltype(&value_t_comparator)
        > cont_{value_t_comparator};
    };
   

 public:
    FindWay(YaRaspCli& cli, YaRaspOutputManager& output_manager,
        CacherType& cache) : ListBase(cli, output_manager), cache_(cache) {  };

 public:
    CommandExeStatus Run() override;

 private:
    void InputParams();

    template<size_t kResultCount>
    nlohmann::json FindPoint(const std::wstring& point_raw_name, const nlohmann::json& point_source_json);

    template<size_t kResultCount>
    std::string FindParam(const std::wstring& point_raw_name);

    size_t GetEditLenght(std::wstring_view src, std::wstring_view temp);
    
 private:
    CacherType& cache_;

 private:
    std::string from_point_id_;
    std::string to_point_id_;
    std::string date_ = "today";    
};


template<typename CacherType>
void FindWay<CacherType>::InputParams() {
    constexpr size_t kResultCount = 10;
    
    while (from_point_id_.empty()) {
        std::wstring from_raw_input;
        output_manager_.GetStreamRef() << "[input <from> point name]> ";
        std::wcin.ignore();
        std::getline(std::wcin, from_raw_input);


        from_point_id_ = FindParam<kResultCount>(from_raw_input);

        if (from_point_id_.empty()) {
            output_manager_.GetStreamRef()
                << "This point is corrupt, ping Yandex Raspisania develop team to fix this problem\n"
                << "Choose another point, repeat request\n"
                << std::endl;
        }
    }

    while (to_point_id_.empty()) {
        std::wstring to_raw_input;
        output_manager_.GetStreamRef() << "[input <to> point name]> ";

        std::wcin.ignore();
        std::getline(std::wcin, to_raw_input);

        to_point_id_ = FindParam<kResultCount>(to_raw_input);

        if (to_point_id_.empty()) {
            output_manager_.GetStreamRef()
                << "This point is corrupt, ping Yandex Raspisania develop team to fix this problem\n"
                << "Choose another point, repeat request\n"
                << std::endl;
        }
    }

    output_manager_.GetStreamRef() << "[input flight date]> ";
    std::cin >> date_;    
}


template<typename CacherType>
template<size_t kResultCount>
std::string FindWay<CacherType>::FindParam(const std::wstring& point_raw_name) {
    nlohmann::json cities = FindPoint<kResultCount>(point_raw_name, cli_.FindCity(""));

    if (cities.size() == 1) {
        if (cities[0].at(YaRaspJsonPtr::kPointId)) {
            return cities[0].at(YaRaspJsonPtr::kPointId);
        } else {
            return "";
        }
    }

    
    nlohmann::json stations = FindPoint<kResultCount>(point_raw_name, cli_.FindStation(""));

    if (stations.size() == 1) {
        if (stations[0].at(YaRaspJsonPtr::kPointId)) {
            return stations[0].at(YaRaspJsonPtr::kPointId);
        } else {
            return "";
        }
    }

    size_t iteration_count = 0;

    output_manager_.GetStreamRef()
        << "list of similar names: " << "\n";
    for (const auto& city : cities) {
        output_manager_.GetStreamRef()
            << "(" << iteration_count << ")" << city.at(YaRaspJsonPtr::kPointName).get_ref<const std::string&>() << "\n";
        ++iteration_count;
    }
    for (const auto& city : cities) {
        output_manager_.GetStreamRef()
            << "(" << iteration_count << ")" << city.at(YaRaspJsonPtr::kPointName).get_ref<const std::string&>() << "\n";
        ++iteration_count;
    }
    output_manager_.GetStreamRef() << std::endl;

    output_manager_.GetStreamRef()
        << "[Choose number of request]> " << "\n";

    size_t chosen_index;
    std::cin >> chosen_index;

    if (chosen_index >= 2 * kResultCount) {
        return "";
    }

    const std::string& result_point_id = (chosen_index < kResultCount)
        ? (cities[chosen_index].contains(YaRaspJsonPtr::kPointId) 
            ? cities[chosen_index].at(YaRaspJsonPtr::kPointId).get_ref<const std::string&>() : "")
        : (stations[chosen_index - kResultCount].contains(YaRaspJsonPtr::kPointId)
            ? stations[chosen_index - kResultCount].at(YaRaspJsonPtr::kPointId).get_ref<const std::string&>() : "");

    return result_point_id;
}


template<typename CacherType>
template<size_t kResultCount>
nlohmann::json FindWay<CacherType>::FindPoint(const std::wstring& point_raw_name, const nlohmann::json& point_source_json) {
    FixedPriorityQueue<kResultCount> result_cont;
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

    for (const nlohmann::json& point : point_source_json) {
        if (point.contains(YaRaspJsonPtr::kPointName)) {
            // std::wstring w_temp = converter.from_bytes(
            //     point.at(YaRaspJsonPtr::kPointName).get<const wchar_t*>()
            // );

            // auto temp_ptr = static_cast<std::wstring>(point.at(YaRaspJsonPtr::kPointName));
            // auto temp_ptr = point.at(YaRaspJsonPtr::kPointName).get<const wchar_t*>();

            size_t edit_lenght = 0;
                // GetEditLenght(point_raw_name, temp_ptr);
            
            if (edit_lenght == 0) {
                return nlohmann::json::array({point});
            } else {
                result_cont.emplace(edit_lenght, point);
            }
        }
    }

    nlohmann::json return_cont = nlohmann::json::array();

    while (result_cont.size()) {
        return_cont.push_back(result_cont.top().second);
        result_cont.pop();
    }

    return return_cont;
}


template<typename CacherType>
size_t FindWay<CacherType>::GetEditLenght(std::wstring_view src, std::wstring_view temp) {
    constexpr const std::array<wchar_t, 12> kSkipedCharCont =
        {' ', '\"', '\'', '\t', '\n', '\r', '-', '(', ')', '\0', ',', '.'};

    auto compare_charapters = [](const wchar_t& lhs, const wchar_t rhs) { return lhs == rhs; };

    auto set_clean_string = [&kSkipedCharCont](std::wstring_view source_string) {
        std::wstring clean_string;
        clean_string.reserve(source_string.size());

        std::for_each(source_string.begin(), source_string.end(), [&clean_string, &kSkipedCharCont](auto& charapter) mutable {
            if (std::find(kSkipedCharCont.begin(), kSkipedCharCont.end(), charapter) == kSkipedCharCont.end()) {
                clean_string.push_back(charapter);
            }
        });

        return clean_string;
    };


    constexpr size_t kAddCost = 1;
    constexpr size_t kDeleteCost = 1;
    constexpr size_t kReplaceCost = 1;
    constexpr size_t kSwapCost = 1;


    std::wstring clean_src = set_clean_string(src);
    std::wstring clean_temp = set_clean_string(temp);

    if (clean_src.empty()) {
        return temp.size() * kAddCost;
    }

    if (clean_temp.empty()) {
        return src.size() * kDeleteCost;
    }

    DynamicContType dyn_cont{clean_src.size() + 1};

    for (size_t index = 1; index < dyn_cont[0].size(); ++index) {
        dyn_cont[0][index] = dyn_cont[0][index - 1] + kAddCost;
    }

    for (int64_t temp_index = 1; temp_index < clean_temp.size() + 1; ++temp_index) {
        for (int64_t src_index = 0; src_index < clean_src.size() + 1; ++src_index) {
            if (src_index) {
                dyn_cont[temp_index][src_index] = std::min (
                std::min (
                    (dyn_cont[temp_index - 1][src_index] + kDeleteCost),
                    (dyn_cont[temp_index][src_index - 1] + kAddCost)
                ),
                std::min (
            (compare_charapters(clean_temp[temp_index - 1], clean_src[src_index - 1]))
                ? (dyn_cont[temp_index - 1][src_index - 1])
                : (dyn_cont[temp_index - 1][src_index - 1] + kReplaceCost),

            (compare_charapters (temp_index > 1 ? clean_temp[temp_index - 2] : '\0', clean_src[src_index])
            && compare_charapters (clean_temp[temp_index], src_index > 1 ? clean_src[src_index - 2] : '\0'))
                ? dyn_cont[temp_index - 2][src_index - 2] + kSwapCost
                : dyn_cont[temp_index - 2][src_index - 2] + kReplaceCost * 2
                    )
                );
            } else {
                dyn_cont[temp_index][src_index] = dyn_cont[temp_index - 1][src_index] + kDeleteCost;
            }
        }
    }

    return dyn_cont[clean_temp.size()][clean_src.size()];
}


template<typename CacherType>
CommandExeStatus FindWay<CacherType>::Run() {
    InputParams();
    return ListWay<CacherType>{cli_, output_manager_, cache_, from_point_id_, to_point_id_, date_}.Run();
}


template<std::derived_from<FindBase> YaRaspListComand, typename CacherType>
class YaRaspApiFindCreator : public ::commands::CommandCreatorBase {
 public:
    YaRaspApiFindCreator(YaRaspCli& cli, YaRaspOutputManager& output_manager,
        CacherType& cache)
            : cli_{cli}, output_manager_{output_manager}, cache_{cache} {  };
 public:
    std::shared_ptr<::commands::CommandBase> Create() override {
        std::string find_of;
        std::cin >> find_of;
        if (find_of == "country") {
            return std::make_shared<FindCountry>(cli_, output_manager_);
        } else if (find_of == "region") {
            return std::make_shared<FindRegion>(cli_, output_manager_);
        } else if (find_of == "city") {
            return std::make_shared<FindCity>(cli_, output_manager_);
        } else if (find_of == "station") {
            return std::make_shared<FindStation>(cli_, output_manager_);
        // } else if (find_of == "way") {
        //     return std::make_shared<FindWay<CacherType>>(cli_, output_manager_, cache_);
        } else {
            return std::make_shared<::commands::InvalidCommand>();
        }
    };
 private:
    YaRaspCli& cli_;
    YaRaspOutputManager& output_manager_;
    CacherType& cache_;
};


} // namespace commands

} // namespace waybuilder


#endif // _APP_COMMANDS_HPP_