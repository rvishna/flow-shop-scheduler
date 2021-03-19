#ifndef OPERATION_REQUIREMENT_GROUP_H
#define OPERATION_REQUIREMENT_GROUP_H

#include "ModelSet.h"
#include "TimeHorizon.h"

namespace flow_shop_scheduler {

class OperationRequirementGroup
{
public:
    std::size_t operationId;
    double fixedCost; // incurred when operation starts

    unsigned int numberOfTimePeriods() const { return numberOfTimePeriods_; }
    double costPerTimePeriod() const { return costPerTimePeriod_; }

    void setDataContext(std::shared_ptr<DataContext>);
    static std::string PathToId();
    static std::string Name(bool plural = false) { return plural ? "operation requirements" : "operation requirement"; }

private:
    duration_type duration_;
    double dailyCost_;

    unsigned int numberOfTimePeriods_;
    double costPerTimePeriod_;

    friend void from_json(const json&, OperationRequirementGroup&);
    friend void to_json(json&, const std::pair<const std::size_t, OperationRequirementGroup>&);
};

void from_json(const json&, OperationRequirementGroup&);
void to_json(json&, const std::pair<const std::size_t, OperationRequirementGroup>&);

struct OperationRequirementGroupSet : public ModelSet<OperationRequirementGroup>
{
    static std::string BasePath();
};

} // namespace flow_shop_scheduler

#endif