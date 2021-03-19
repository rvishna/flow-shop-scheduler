#include "OperationRequirementGroup.h"

namespace flow_shop_scheduler {

static const std::string PathToDuration = "duration";
static const std::string PathToOperationId = "operation";
#ifdef USE_CAMEL_CASE
static const std::string PathToFixedCost = "fixedCost";
static const std::string PathToDailyCost = "dailyCost";
#else
static const std::string PathToFixedCost = "fixed_cost";
static const std::string PathToDailyCost = "daily_cost";
#endif

static const double DefaultFixedCost = 0;
static const double DefaultDailyCost = 0;

std::string OperationRequirementGroup::PathToId()
{
    return "id";
}

std::string OperationRequirementGroupSet::BasePath()
{
#ifdef USE_CAMEL_CASE
    return "/operationRequirementGroups";
#else
    return "/operation_requirement_groups";
#endif
}

void OperationRequirementGroup::setDataContext(std::shared_ptr<DataContext> dataContext)
{
    assert(dataContext);
    const auto& timeHorizon = dataContext->get<TimeHorizon>();
    costPerTimePeriod_ = dailyCost_ * timeHorizon.daysPerTimePeriod();
    numberOfTimePeriods_ = duration_ / timeHorizon.timePeriodDuration;
    if(numberOfTimePeriods_ * timeHorizon.timePeriodDuration < duration_)
        numberOfTimePeriods_++;
}

void to_json(json& j, const std::pair<const std::size_t, OperationRequirementGroup>& operationRequirement)
{
    j[OperationRequirementGroup::PathToId()] = operationRequirement.first;
    j[PathToOperationId] = operationRequirement.second.operationId;
    j[PathToDuration] = operationRequirement.second.duration_;
    j[PathToFixedCost] = operationRequirement.second.fixedCost;
    j[PathToDailyCost] = operationRequirement.second.dailyCost_;
}

void from_json(const json& j, OperationRequirementGroup& operationRequirement)
{
    j.at(PathToOperationId).get_to(operationRequirement.operationId);
    j.at(PathToDuration).get_to(operationRequirement.duration_);
    operationRequirement.fixedCost = j.value(PathToFixedCost, json{}).get<std::optional<double>>().value_or(DefaultFixedCost);
    operationRequirement.dailyCost_ = j.value(PathToDailyCost, json{}).get<std::optional<double>>().value_or(DefaultDailyCost);
}

} // namespace flow_shop_scheduler