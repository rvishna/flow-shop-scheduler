#include "Machine.h"

namespace flow_shop_scheduler {

static const std::string PathToName = "name";

#ifdef USE_CAMEL_CASE
static const std::string PathToStartTime = "startTime";
static const std::string PathToEndTime = "endTime";
#else
static const std::string PathToStartTime = "start_time";
static const std::string PathToEndTime = "end_time";
#endif

std::string Machine::PathToId()
{
    return "id";
}

void Machine::setDataContext(std::shared_ptr<DataContext> dataContext)
{
    assert(dataContext);
    const auto& timeHorizon = dataContext->get<TimeHorizon>();
    startTimePeriod_ = startTime_ ? timeHorizon.getEarliestPeriodStartsAfter(startTime_.value()) : 0;
    endTimePeriod_ = endTime_ ? timeHorizon.getLatestPeriodEndsBefore(endTime_.value()) : timeHorizon.numberOfTimePeriods() - 1;
}

void to_json(json& j, const std::pair<const std::size_t, Machine>& machine)
{
    j[Machine::PathToId()] = machine.first;
    j[PathToName] = machine.second.name;
    j[PathToStartTime] = machine.second.startTime_;
    j[PathToEndTime] = machine.second.endTime_;
}

void from_json(const json& j, Machine& machine)
{
    j.at(PathToName).get_to(machine.name);
    j.value(PathToStartTime, json{}).get_to(machine.startTime_);
    j.value(PathToEndTime, json{}).get_to(machine.endTime_);
}

} // namespace flow_shop_scheduler