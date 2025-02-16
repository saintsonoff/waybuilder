#ifndef _COMMAND_HPP_
#define _COMMAND_HPP_

#include <concepts>
#include <memory>

namespace commands {
  
  enum class CommandExeStatus { CORRECT, INVALID_INPUT, FAIL, EXIT };
  
  class CommandBase {
    public:
    virtual CommandExeStatus Run() = 0;
    virtual ~CommandBase() = default;
  };
  
  class InvalidCommand : public CommandBase {
    public:
    InvalidCommand() = default;
    CommandExeStatus Run() override { return CommandExeStatus::INVALID_INPUT; };
    ~InvalidCommand() override = default;
  };
  
  class CommandCreatorBase {
    public:
    virtual std::shared_ptr<CommandBase> Create() = 0;
    ~CommandCreatorBase() = default;
  };
  
  template<std::derived_from<CommandBase> CommandType>
  class CommandCreator : public CommandCreatorBase {
    public:
    std::shared_ptr<CommandBase> Create() override {
      return std::make_shared<CommandType>();
    };
    
    ~CommandCreator() = default;
  };
  
} // namespace commands

#endif // _COMMAND_HPP_
  