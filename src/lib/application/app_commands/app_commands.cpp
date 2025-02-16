#include "app_commands.hpp"

#include <string_view>
#include <memory>
#include <iostream>

namespace waybuilder {

namespace commands {

std::string_view Help::kHelpText = "help command";


CommandExeStatus Save::Run() {
    if (cli_.Save()) {
        std::cout << "Save is done" << std::endl;
    } else {
        std::cout << "Cannot save config or points, check path" << std::endl;
    }

    return CommandExeStatus::CORRECT;
}

} // namespace commands

} // namespace waybuilder