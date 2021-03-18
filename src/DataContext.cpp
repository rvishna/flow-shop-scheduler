#include "DataContext.h"

namespace flow_shop_scheduler {

void JSONWriter::write(std::ostream& os)
{
    os << j_;
}

void JSONWriter::set(const json::json_pointer& p, const JSONSerializable& s)
{
    s.set(j_[p]);
}

void JSONParser::parse(std::istream& is)
{
    is >> j_;
}

void JSONParser::get(const json::json_pointer& p, JSONSerializable& s, std::shared_ptr<DataContext> oc) const
{
    if(j_.contains(p))
    {
        s.get(j_[p], oc);
    }
    else
    {
        std::ostringstream oss;
        oss << "Missing element at path " << p << ".";
        throw Exception(oss.str());
    }
}

void DataContext::parse(std::istream& is)
{
    if(cache_.empty())
    {
        std::ostringstream oss;
        oss << is.rdbuf();
        cache_ = oss.str();
    }

    std::istringstream iss(cache_);
    JSONParser parser;
    parser.parse(iss);
    for(const auto& i : parseOrder_)
        parser.get(json::json_pointer(i), *data_[i], shared_from_this());
}

void DataContext::write(std::ostream& os) const
{
    JSONWriter writer;
    for(const auto& i : parseOrder_)
        writer.set(json::json_pointer(i), *data_.at(i));
    writer.write(os);
}

void DataContext::clear()
{
    data_.clear();
    parseOrder_.clear();
    resetCache();
}

void DataContext::resetCache()
{
    cache_.clear();
}

} // namespace flow_shop_scheduler
