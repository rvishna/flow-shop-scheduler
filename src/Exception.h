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

struct MandatoryFieldMissing : public Exception
{
    MandatoryFieldMissing(const char* message)
    : Exception(message)
    {
    }

    MandatoryFieldMissing(const std::string& message)
    : Exception(message)
    {
    }

    static MandatoryFieldMissing Create(const std::string& name, const json& j, const std::string& message)
    {
        std::ostringstream oss;
        oss << "Missing a mandatory field when deserializing " << name << std::endl
            << std::setw(4) << j << std::endl
            << "Parser reported error: " << message;
        return MandatoryFieldMissing(oss.str());
    }
};

struct InvalidType : public Exception
{
    InvalidType(const char* message)
    : Exception(message)
    {
    }
    InvalidType(const std::string& message)
    : Exception(message)
    {
    }

    static InvalidType Create(const std::string& name, const json& j, const std::string& message)
    {
        std::ostringstream oss;
        oss << "Invalid type for field when deserializing " << name << std::endl
            << std::setw(4) << j << std::endl
            << "Parser reported error: " << message;
        return InvalidType(oss.str());
    }
};

struct DuplicatePrimaryKey : public Exception
{
    DuplicatePrimaryKey(const char* message)
    : Exception(message)
    {
    }
    DuplicatePrimaryKey(const std::string& message)
    : Exception(message)
    {
    }

    template<typename T>
    static DuplicatePrimaryKey Create(std::size_t id, const std::pair<const std::size_t, T>& existing_value, const std::pair<const std::size_t, T>& new_value)
    {
        std::ostringstream oss;
        oss << "Duplicate value " << id << " for primary key field \"" << T::PathToId() << "\" for " << T::Name(true) << ":" << std::endl
            << std::setw(4) << json(existing_value) << std::endl
            << "and" << std::endl
            << std::setw(4) << json(new_value);
        return DuplicatePrimaryKey(oss.str());
    }
};

} // namespace flow_shop_scheduler

#endif
