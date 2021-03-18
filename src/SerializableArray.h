#ifndef SERIALIZABLE_ARRAY_H
#define SERIALIZABLE_ARRAY_H

#include "DataContext.h"

namespace flow_shop_scheduler {

template<typename T>
class SerializableArray : public JSONSerializable
{
public:
    virtual void set(json&) const override;
    virtual void get(const json&, std::shared_ptr<DataContext> dataContext = nullptr) override;

    std::size_t size() const { return data_.size(); }
    bool empty() const { return data_.empty(); }

    T& operator[](std::size_t i) { return data_[i]; }
    const T& operator[](std::size_t i) const { return data_[i]; }

    typedef typename std::vector<T>::iterator iterator;
    typedef typename std::vector<T>::const_iterator const_iterator;

    iterator begin() noexcept { return data_.begin(); }
    iterator end() noexcept { return data_.end(); }

    const_iterator begin() const noexcept { return data_.begin(); }
    const_iterator end() const noexcept { return data_.end(); }

    void push_back(const T& val) { data_.push_back(val); }

private:
    std::vector<T> data_;
};

template<typename T>
void SerializableArray<T>::set(json& j) const
{
    j = data_;
}

template<typename T>
void SerializableArray<T>::get(const json& j, std::shared_ptr<DataContext> dataContext)
{
    if(!j.is_array())
    {
        std::ostringstream oss;
        oss << "Deserializing to " << T::Name(true) << " requires a JSON array.";
        throw Exception(oss.str());
    }

    for(const auto& o : j)
    {
        try
        {
            data_.push_back(o.get<T>());
        }
        catch(const json::type_error& e)
        {
            if(e.id == 302)
                throw InvalidType::Create(T::Name(), o, e.what());
            else
                throw;
        }
        catch(const json::out_of_range& e)
        {
            if(e.id == 403)
                throw MandatoryFieldMissing::Create(T::Name(), o, e.what());
            else
                throw;
        }

        if(dataContext)
            data_.back().setDataContext(dataContext);
    }
}

} // namespace flow_shop_scheduler

#endif