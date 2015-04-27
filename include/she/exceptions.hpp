#pragma once

#include <exception>
#include <string>
#include <sstream>


namespace she
{

class precondition_not_satisfied : public std::exception
{
 public:
    precondition_not_satisfied(const std::string & msg) noexcept : _msg(msg) {};
    const char * what() const noexcept { return _msg.c_str(); }

 private:
    std::string _msg;
};

} // namespace she


#define ASSERT( condition, msg ) { \
    if (!(condition)) { \
        std::stringstream ss; \
        ss << msg << " (" << #condition << ")"; \
        throw she::precondition_not_satisfied(ss.str()); \
    } }
