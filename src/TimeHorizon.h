#ifndef TIME_HORIZON_H
#define TIME_HORIZON_H

#include <chrono>

#include <date/date.h>
#include <date/tz.h>

#include "DataContext.h"

namespace flow_shop_scheduler {

using nlohmann::json;

using duration_type = std::chrono::duration<int64_t, std::ratio<1, 1000000>>; // 1 microsecond
using time_type = date::sys_time<duration_type>;

class TimeHorizon : public JSONSerializable
{
public:
    static std::string BasePath();

    void set(json&) const override;
    void get(const json&, std::shared_ptr<DataContext> dataContext = nullptr) override;

    time_type startTime;

    void setEndTime(time_type endTime);

    duration_type timePeriodDuration;

    virtual time_type getPeriodStartTime(int timePeriod) const;
    virtual time_type getPeriodEndTime(int timePeriod) const;

    virtual int getLatestPeriodEndsBefore(time_type t) const;    // latest time period that ends at or before t
    virtual int getEarliestPeriodStartsAfter(time_type t) const; // earliest time period that starts at or after t
    virtual int getLatestPeriodStartsBefore(time_type t) const;  // latest time period that starts at or before t
    virtual int getEarliestPeriodEndsAfter(time_type t) const;   // earliest time period that ends at or after t

    virtual int numberOfTimePeriods() const;
    virtual double daysPerTimePeriod() const;

private:
    time_type adjustedEndTime_;

    friend void from_json(const json&, TimeHorizon&);
    friend void to_json(json&, const TimeHorizon&);
};

void from_json(const json&, TimeHorizon&);
void to_json(json&, const TimeHorizon&);

} // namespace flow_shop_scheduler

namespace nlohmann {

template<>
struct adl_serializer<flow_shop_scheduler::time_type>
{
    static void to_json(json&, const flow_shop_scheduler::time_type&);
    static void from_json(const json&, flow_shop_scheduler::time_type&);
};

template<>
struct adl_serializer<flow_shop_scheduler::duration_type>
{
    static void to_json(json&, const flow_shop_scheduler::duration_type&);
    static void from_json(const json&, flow_shop_scheduler::duration_type&);
};

} // namespace nlohmann

#endif