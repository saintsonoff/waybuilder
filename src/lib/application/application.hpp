#ifndef _APPLICATION_HPP_
#define _APPLICATION_HPP_

namespace waybuilder {

enum ApplicationCategories { CONSOLE_APP, CONSOLE_CLI, GUI };

enum ExitStatus { CORRECT = 0, FAIL = 1 };

namespace __detail {

template<ApplicationCategories kCategory>
class Application {};

} // namespace __detail

} // namespace waybuilder

#endif // _APPLICATION_HPP_
