#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <exception>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include <nlohmann/json.hpp>

namespace flow_shop_scheduler {

using nlohmann::json;

class Exception : public std::exception
{
public:
    Exception(const char* message)
    : message_(message)
    {
    }

    Exception(const std::string& message)
    : message_(message)
    {
    }

    Exception(const Exception& e)
    : std::exception(e)
    , message_(e.message_)
    {
    }

    Exception& operator=(const Exception& e)
    {
        message_ = e.message_;
        return *this;
    }

    const char* what() const noexcept { return message_.c_str(); }

private:
    std::string message_;
};

} // namespace flow_shop_scheduler

#endif
