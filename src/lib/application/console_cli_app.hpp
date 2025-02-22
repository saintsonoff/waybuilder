#ifndef _CONSOLE_CLI_APP_HPP_
#define _CONSOLE_CLI_APP_HPP_

#include "application.hpp"

#include <cstddef>
#include <string_view>

#include <command_fab.hpp>
#include <ya_rasp_cli.hpp>
#include <output_manager.hpp>
#include <lru_cache.hpp>

namespace waybuilder {

namespace __detail {
    
template<>
class Application<ApplicationCategories::CONSOLE_CLI> {
 public:
   static constexpr size_t kCacheSize = 50;
   using CacheType = LruCache<std::string, nlohmann::json, kCacheSize>;
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
    CacheType cache_;
};

} // namespace __detail

using ConsoleWayBuilderApp = __detail::Application<ApplicationCategories::CONSOLE_CLI>;

} // namespace waybuilder

#endif // _CONSOLE_CLI_APP_HPP_
