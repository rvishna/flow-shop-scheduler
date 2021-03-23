#include "OperationRequirement.h"

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

void OperationRequirement::setDataContext(std::shared_ptr<DataContext> dataContext)
{
    assert(dataContext);
    const auto& timeHorizon = dataContext->get<TimeHorizon>();
    costPerTimePeriod_ = dailyCost_ * timeHorizon.daysPerTimePeriod();
    numberOfTimePeriods_ = duration_ / timeHorizon.timePeriodDuration;
    if(numberOfTimePeriods_ * timeHorizon.timePeriodDuration < duration_)
        numberOfTimePeriods_++;
}

void to_json(json& j, const OperationRequirement& operationRequirement)
{
    j[PathToOperationId] = operationRequirement.operationId;
    j[PathToDuration] = operationRequirement.duration_;
    j[PathToFixedCost] = operationRequirement.fixedCost;
    j[PathToDailyCost] = operationRequirement.dailyCost_;
}

void from_json(const json& j, OperationRequirement& operationRequirement)
{
    j.at(PathToOperationId).get_to(operationRequirement.operationId);
    j.at(PathToDuration).get_to(operationRequirement.duration_);
    operationRequirement.fixedCost = j.value(PathToFixedCost, json{}).get<std::optional<double>>().value_or(DefaultFixedCost);
    operationRequirement.dailyCost_ = j.value(PathToDailyCost, json{}).get<std::optional<double>>().value_or(DefaultDailyCost);
}

} // namespace flow_shop_scheduler