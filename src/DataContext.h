#ifndef DATA_CONTEXT_H
#define DATA_CONTEXT_H

#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

#include "Exception.h"

namespace flow_shop_scheduler {

using nlohmann::json;

class DataContext; // container class for dependency injection

struct JSONSerializable
{
    virtual ~JSONSerializable() {}

    virtual void get(const json&, std::shared_ptr<DataContext>) = 0;
    virtual void set(json&) const = 0;
};

class JSONParser
{
public:
    void parse(std::istream&);
    void get(const json::json_pointer&, JSONSerializable&, std::shared_ptr<DataContext> oc) const;

private:
    json j_;
};

class JSONWriter
{
public:
    void write(std::ostream&);
    void set(const json::json_pointer&, const JSONSerializable&);

private:
    json j_;
};

class DataContext : public std::enable_shared_from_this<DataContext>
{
public:
    template<typename T>
    void add(std::unique_ptr<T>);

    template<typename T>
    const T& get() const;

    template<typename T>
    T& get() { return const_cast<T&>(const_cast<const DataContext*>(this)->get<T>()); }

    template<typename T>
    bool contains() const;

    void parse(std::istream& is);
    void write(std::ostream& os) const;
    void clear();
    void resetCache();

private:
    std::unordered_map<std::string, std::unique_ptr<JSONSerializable>> data_;
    std::vector<std::string> parseOrder_;
    std::string cache_;
};

template<typename T>
void DataContext::add(std::unique_ptr<T> s)
{
    const auto& path = T::BasePath();

    if(data_.count(path) != 0)
    {
        assert(std::find(parseOrder_.begin(), parseOrder_.end(), path) != parseOrder_.end());
        std::ostringstream oss;
        oss << "Path '" << path << "' is already registered in data context.";
        throw Exception(oss.str());
    }
    else
    {
        data_[path] = std::move(s);
        parseOrder_.push_back(path);
    }
}

template<typename T>
bool DataContext::contains() const
{
    const std::string& path = T::BasePath();
    return data_.count(path);
}

template<typename T>
const T& DataContext::get() const
{
    const std::string& path = T::BasePath();
    auto it = data_.find(path);

    if(it == data_.end())
    {
        std::ostringstream oss;
        oss << "No object is registered to path '" << path << "' in data context.";
        throw Exception(oss.str());
    }
    else
    {
        T* p = dynamic_cast<T*>(it->second.get());
        if(!p)
        {
            std::ostringstream oss;
            oss << "Type mismatch for object registered to path '" << path << "'.";
            throw Exception(oss.str());
        }
        else
        {
            return *p;
        }
    }
}

} // namespace flow_shop_scheduler

namespace nlohmann {

template<typename T>
struct adl_serializer<std::optional<T>>
{
    static void from_json(const json& j, std::optional<T>& t)
    {
        if(j.is_null())
            t = std::nullopt;
        else
            t = j.get<T>();
    }

    static void to_json(json& j, const std::optional<T>& t)
    {
        if(t)
            j = t.value();
        else
            j = nullptr;
    }
};

} // namespace nlohmann

#endif