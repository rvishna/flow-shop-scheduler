#include "SimulatorOptions.h"

namespace flow_shop_scheduler {

static const std::string PathToStrategy = "strategy";

std::string SimulatorOptions::BasePath()
{
#ifdef USE_CAMEL_CASE
    return "/simulatorOptions";
#else
    return "/simulator_options";
#endif
}

void SimulatorOptions::set(json& j) const
{
    to_json(j, *this);
}

void SimulatorOptions::get(const json& j, std::shared_ptr<DataContext>)
{
    from_json(j, *this);
}

void to_json(json& j, const SimulatorOptions& simulatorOptions)
{
    j[PathToStrategy] = simulatorOptions.strategy;
}

void from_json(const json& j, SimulatorOptions& simulatorOptions)
{
    j.at(PathToStrategy).get_to(simulatorOptions.strategy);
}

} // namespace flow_shop_scheduler
