#ifndef SIMULATION_STRATEGY_H
#define SIMULATION_STRATEGY_H

#include "ScheduleEvent.h"

namespace flow_shop_scheduler {

struct SimulationStrategy
{
    virtual ~SimulationStrategy();
    virtual Schedule generate() = 0;
    virtual void setDataContext(std::shared_ptr<DataContext>) = 0;

    static std::unique_ptr<SimulationStrategy> Factory(const std::string&);
};

} // namespace flow_shop_scheduler

#endif