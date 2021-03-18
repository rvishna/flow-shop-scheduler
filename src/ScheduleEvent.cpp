#include "ScheduleEvent.h"

namespace flow_shop_scheduler {

static const std::string PathToId = "id";
static const std::string PathToDuration = "duration";
static const std::string PathToJobId = "job";
static const std::string PathToOperationId = "operation";
static const std::string PathToMachineId = "machine";

#ifdef USE_CAMEL_CASE
static const std::string PathToStartTime = "startTime";
#else
static const std::string PathToStartTime = "start_time";
#endif

std::string Schedule::BasePath()
{
    return "/schedule";
}

ScheduleEvent::ScheduleEvent(std::shared_ptr<DataContext> dataContext, std::size_t operationId, std::size_t jobId, std::optional<std::size_t> machineId, int startTimePeriod, unsigned int numberOfTimePeriods)
: dataContext_(dataContext)
, operationId(operationId)
, jobId(jobId)
, machineId(machineId)
{
    setStartTimePeriod(startTimePeriod);
    setNumberOfTimePeriods(numberOfTimePeriods);
}

void ScheduleEvent::setStartTimePeriod(int startTimePeriod)
{
    assert(dataContext_ && dataContext_->contains<TimeHorizon>());
    startTimePeriod_ = startTimePeriod;
    startTime_ = dataContext_->get<TimeHorizon>().getPeriodStartTime(startTimePeriod_);
}

void ScheduleEvent::setNumberOfTimePeriods(unsigned int numberOfTimePeriods)
{
    assert(dataContext_ && dataContext_->contains<TimeHorizon>());
    numberOfTimePeriods_ = numberOfTimePeriods;
    duration_ = numberOfTimePeriods_ * dataContext_->get<TimeHorizon>().timePeriodDuration;
}

bool ScheduleEvent::isOngoingBetween(int otherStartTimePeriod, int otherEndTimePeriod) const
{
    return !(startTimePeriod_ > otherEndTimePeriod || startTimePeriod_ + static_cast<int>(numberOfTimePeriods_) <= otherStartTimePeriod);
}

void ScheduleEvent::setDataContext(std::shared_ptr<DataContext> dataContext)
{
    dataContext_ = dataContext;
    if(dataContext_->contains<TimeHorizon>())
    {
        const auto& timeHorizon = dataContext_->get<TimeHorizon>();
        startTimePeriod_ = timeHorizon.getLatestPeriodStartsBefore(startTime_);
        numberOfTimePeriods_ = duration_ / timeHorizon.timePeriodDuration;
        if(timeHorizon.timePeriodDuration * numberOfTimePeriods_ < duration_)
            numberOfTimePeriods_++;
    }
}

void to_json(json& j, const ScheduleEvent& scheduleEvent)
{
    j[PathToJobId] = scheduleEvent.jobId;
    j[PathToOperationId] = scheduleEvent.operationId;
    j[PathToStartTime] = scheduleEvent.startTime_;
    j[PathToMachineId] = scheduleEvent.machineId;
    j[PathToDuration] = scheduleEvent.duration_;
}

void from_json(const json& j, ScheduleEvent& scheduleEvent)
{
    j.at(PathToJobId).get_to(scheduleEvent.jobId);
    j.at(PathToOperationId).get_to(scheduleEvent.operationId);
    j.at(PathToStartTime).get_to(scheduleEvent.startTime_);
    j.value(PathToMachineId, json{}).get_to(scheduleEvent.machineId);
    j.value(PathToDuration, json{}).get_to(scheduleEvent.duration_);
}

} // namespace flow_shop_scheduler