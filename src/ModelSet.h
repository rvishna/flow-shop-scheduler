#ifndef MODEL_SET_H
#define MODEL_SET_H

#include "DataContext.h"

namespace flow_shop_scheduler {

template<typename T>
class ModelSet : public JSONSerializable
{
public:
    virtual void set(json&) const override;
    virtual void get(const json&, std::shared_ptr<DataContext> dataContext = nullptr) override;

    virtual void setDataContext(std::shared_ptr<DataContext>){};

    int count(std::size_t key) const { return data_.count(key); }
    std::size_t size() const { return data_.size(); }
    bool empty() const { return data_.empty(); }

    T& operator[](std::size_t key) { return data_[key]; }
    const T& operator[](std::size_t key) const { return data_.at(key); }

    typedef typename std::unordered_map<std::size_t, T>::iterator iterator;
    typedef typename std::unordered_map<std::size_t, T>::const_iterator const_iterator;

    typedef typename std::unordered_map<std::size_t, T>::value_type value_type;

    iterator insertUnique(const value_type& value);

    iterator begin() noexcept { return data_.begin(); }
    iterator end() noexcept { return data_.end(); }

    const_iterator begin() const noexcept { return data_.begin(); }
    const_iterator end() const noexcept { return data_.end(); }

    iterator find(std::size_t key) { return data_.find(key); }
    const_iterator find(std::size_t key) const { return data_.find(key); }

private:
    std::unordered_map<std::size_t, T> data_;
};

template<typename T>
typename ModelSet<T>::iterator ModelSet<T>::insertUnique(const ModelSet<T>::value_type& value)
{
    auto p = data_.insert(value);
    if(!p.second)
        throw DuplicatePrimaryKey::Create(value.first, *p.first, value);
    return p.first;
}

template<typename T>
void ModelSet<T>::set(json& j) const
{
    j = data_;
}

template<typename T>
void ModelSet<T>::get(const json& j, std::shared_ptr<DataContext> dataContext)
{
    if(!j.is_array())
    {
        std::ostringstream oss;
        oss << "Deserializing to " << T::Name(true) << " requires a JSON array.";
        throw Exception(oss.str());
    }

    const auto& pathToId = T::PathToId();

    data_.clear();

    setDataContext(dataContext);

    for(const auto& o : j)
    {
        if(!o.contains(pathToId))
        {
            std::ostringstream oss;
            oss << "primary key \"" << pathToId << "\" not found";
            throw MandatoryFieldMissing::Create(T::Name(), o, oss.str());
        }

        std::size_t id;
        try
        {
            o[pathToId].get_to(id);
        }
        catch(const json::type_error& e)
        {
            if(e.id == 302)
                throw InvalidType::Create(T::Name(), o, e.what());
            else
                throw;
        }

        try
        {
            insertUnique(std::make_pair(id, o.get<T>()));
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
            data_[id].setDataContext(dataContext);
    }
}

} // namespace flow_shop_scheduler

#endif