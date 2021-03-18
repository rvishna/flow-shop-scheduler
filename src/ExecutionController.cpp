#include <spdlog/spdlog.h>

#include "DataContext.h"
#include "ExecutionController.h"
#include "Job.h"
#include "Operation.h"
#include "OperationRequirementGroup.h"
#include "SimulationStrategy.h"
#include "SimulatorOptions.h"

namespace flow_shop_scheduler {

struct ExecutionController::ExecutionControllerImpl
{
public:
    ExecutionControllerImpl(std::istream& is, std::ostream& os);
    void execute() const;

private:
    std::shared_ptr<DataContext> inputContext_;
    std::istream& is_;
    std::ostream& os_;
    std::unique_ptr<SimulationStrategy> simulationStrategy_;
};

ExecutionController::ExecutionControllerImpl::ExecutionControllerImpl(std::istream& is, std::ostream& os)
: is_(is)
, os_(os)
, inputContext_(new DataContext)
{
    inputContext_->add(std::make_unique<SimulatorOptions>());
    inputContext_->parse(is);
    simulationStrategy_ = SimulationStrategy::Factory(inputContext_->get<SimulatorOptions>().strategy);
    inputContext_->add(std::make_unique<TimeHorizon>());
    inputContext_->add(std::make_unique<OperationSet>());
    inputContext_->add(std::make_unique<OperationRequirementGroupSet>());
    inputContext_->add(std::make_unique<JobSet>());
    inputContext_->add(std::make_unique<Schedule>());
    simulationStrategy_->setDataContext(inputContext_);

    spdlog::info("Starting flow shop scheduler with {} operations, {} jobs, {} time periods", inputContext_->get<OperationSet>().size(), inputContext_->get<JobSet>().size(), inputContext_->get<TimeHorizon>().numberOfTimePeriods());
}

void ExecutionController::ExecutionControllerImpl::execute() const
{
    auto schedule = simulationStrategy_->generate();
    spdlog::info("Generated schedule with {} events using strategy '{}'", schedule.size(), inputContext_->get<SimulatorOptions>().strategy);
    json j;
    schedule.set(j);
    os_ << j;
}

ExecutionController::ExecutionController(std::istream& is, std::ostream& os)
: pimpl_(new ExecutionControllerImpl(is, os))
{
}

ExecutionController::~ExecutionController() = default;

void ExecutionController::execute() const
{
    pimpl_->execute();
}

} // namespace flow_shop_scheduler
