#include "SimulationStrategy.h"
#include "PreferredSequenceConstructiveHeuristic.h"

namespace flow_shop_scheduler {

SimulationStrategy::~SimulationStrategy()
{
}

std::unique_ptr<SimulationStrategy> SimulationStrategy::Factory(const std::string& strategy)
{
    if(strategy == "preferred_sequence_constructive_heuristic")
        return std::make_unique<PreferredSequenceConstructiveHeuristicStrategy>();
    std::ostringstream oss;
    oss << "Invalid simulation strategy '" << strategy << "'.";
    throw Exception(oss.str());
}

} // namespace flow_shop_scheduler
