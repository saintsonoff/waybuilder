#ifndef _COMMAND_FAB_HPP_
#define _COMMAND_FAB_HPP_

#include <map>
#include <algorithm>
#include <memory>
#include <istream>
#include <type_traits>
#include <utility>
#include <string>
#include <concepts>
#include <memory>

#include "command_module.hpp"

namespace commands {

class CommandFabric {
 public:
  template<std::derived_from<CommandCreatorBase> CommandCreatorType>
  bool Add(std::pair<std::string, CommandCreatorType>&& creator_value) {
    return creator_container_.insert(
      {creator_value.first, std::make_shared<CommandCreatorType>(creator_value.second)}
    ).second;
  };

  template<std::derived_from<CommandCreatorBase> CommandCreatorType>
  bool Add(const std::pair<std::string, CommandCreatorType>& creator_value) {
    return creator_container_.insert(creator_value).second;
  };

  template<std::derived_from<CommandCreatorBase>... CommandCreatorTypes>
  bool Add(const std::pair<std::string, CommandCreatorTypes>&... creator_values) {
    return (creator_container_.insert(
      {creator_values.first, std::make_shared<std::decay_t<CommandCreatorTypes>>(creator_values.second)}
    ).second && ...);
  };

 public:

  std::shared_ptr<CommandBase> GetCommand(const std::string& key) {
    if (auto creator_itr = creator_container_.find(key);
      creator_itr != creator_container_.end()) {
        return creator_itr->second->Create();
    } else {
      return std::shared_ptr<InvalidCommand>{};
    }
  };

  std::shared_ptr<CommandBase> GetCommand(std::istream& stream) {
    std::string key_buff;

    stream >> key_buff;

    if (stream.fail())
      return nullptr;

    if (auto creator_itr = creator_container_.find(key_buff);
      creator_itr != creator_container_.end()) {
        return creator_itr->second->Create();
    } else {
      return std::make_shared<InvalidCommand>();
    }
  };

 private:
  std::map<std::string, std::shared_ptr<CommandCreatorBase>> creator_container_;
};

} // namespace commands

#endif // _COMMAND_FAB_HPP_