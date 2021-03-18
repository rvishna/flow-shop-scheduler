#include "Job.h"

namespace flow_shop_scheduler {

static const std::string PathToName = "name";

#ifdef USE_CAMEL_CASE
static const std::string PathToStartTime = "startTime";
static const std::string PathToOperationRequirementGroupId = "operationRequirementGroupId";
#else
static const std::string PathToStartTime = "start_time";
static const std::string PathToOperationRequirementGroupId = "operation_requirement_group_id";
#endif

std::string Job::PathToId()
{
    return "id";
}

std::string JobSet::BasePath()
{
    return "/jobs";
}

void Job::setDataContext(std::shared_ptr<DataContext> dataContext)
{
    if(dataContext->contains<TimeHorizon>())
    {
        const auto& timeHorizon = dataContext->get<TimeHorizon>();
        startTimePeriod_ = startTime_ ? timeHorizon.getEarliestPeriodStartsAfter(startTime_.value()) : 0;
    }
}

void to_json(json& j, const std::pair<const std::size_t, Job>& job)
{
    j[Job::PathToId()] = job.first;
    j[PathToName] = job.second.name;
    j[PathToOperationRequirementGroupId] = job.second.operationRequirementGroupId;
    j[PathToStartTime] = job.second.startTime_;
}

void from_json(const json& j, Job& job)
{
    j.at(PathToName).get_to(job.name);
    j.at(PathToOperationRequirementGroupId).get_to(job.operationRequirementGroupId);
    j.value(PathToStartTime, json{}).get_to(job.startTime_);
}

} // namespace flow_shop_scheduler