#ifndef SCHEDULE_EVENT_H
#define SCHEDULE_EVENT_H

#include "SerializableArray.h"
#include "TimeHorizon.h"

namespace flow_shop_scheduler {

class ScheduleEvent
{
public:
    ScheduleEvent() = default;
    ScheduleEvent(std::shared_ptr<DataContext>, std::size_t, std::size_t, std::optional<std::size_t>, int, unsigned int);

    std::size_t jobId;
    std::size_t operationId;
    std::optional<std::size_t> machineId;

    time_type startTime() const { return startTime_; }
    duration_type duration() const { return duration_; }

    int startTimePeriod() const { return startTimePeriod_; }
    int endTimePeriod() const { return startTimePeriod_ + numberOfTimePeriods_ - 1; }
    int numberOfTimePeriods() const { return numberOfTimePeriods_; }

    void setStartTimePeriod(int);
    void setNumberOfTimePeriods(unsigned int);

    bool isOngoingBetween(int, int) const;

    void setDataContext(std::shared_ptr<DataContext>);
    static std::string Name(bool plural = false) { return plural ? "schedule events" : "schedule event"; }

private:
    std::shared_ptr<DataContext> dataContext_;

    int startTimePeriod_;
    unsigned int numberOfTimePeriods_;

    time_type startTime_;
    duration_type duration_;

    friend void from_json(const json&, ScheduleEvent&);
    friend void to_json(json&, const ScheduleEvent&);
};

void from_json(const json&, ScheduleEvent&);
void to_json(json&, const ScheduleEvent&);

class Schedule : public SerializableArray<ScheduleEvent>
{
public:
    static std::string BasePath();

private:
    std::shared_ptr<DataContext> dataContext_;
};

} // namespace flow_shop_scheduler

#endif