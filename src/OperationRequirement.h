#ifndef OPERATION_REQUIREMENT_H
#define OPERATION_REQUIREMENT_H

#include "TimeHorizon.h"

namespace flow_shop_scheduler {

class OperationRequirement
{
public:
    std::size_t operationId;
    double fixedCost; // incurred when operation starts

    unsigned int numberOfTimePeriods() const { return numberOfTimePeriods_; }
    double costPerTimePeriod() const { return costPerTimePeriod_; }

    void setDataContext(std::shared_ptr<DataContext>);
    static std::string Name(bool plural = false) { return plural ? "operation requirements" : "operation requirement"; }

private:
    duration_type duration_;
    double dailyCost_;

    unsigned int numberOfTimePeriods_;
    double costPerTimePeriod_;

    friend void from_json(const json&, OperationRequirement&);
    friend void to_json(json&, const OperationRequirement&);
};

void from_json(const json&, OperationRequirement&);
void to_json(json&, const OperationRequirement&);

} // namespace flow_shop_scheduler

#endif