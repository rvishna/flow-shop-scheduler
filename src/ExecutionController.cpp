#include <spdlog/spdlog.h>

#include "ExecutionController.h"

namespace flow_shop_scheduler {

struct ExecutionController::ExecutionControllerImpl
{
public:
    ExecutionControllerImpl(std::istream& is, std::ostream& os);
    void execute() const;

private:
    std::istream& is_;
    std::ostream& os_;
};

ExecutionController::ExecutionControllerImpl::ExecutionControllerImpl(std::istream& is, std::ostream& os)
: is_(is)
, os_(os)
{
    spdlog::info("Starting job");
}

void ExecutionController::ExecutionControllerImpl::execute() const
{
    os_ << "{}";
    spdlog::info("Finished job");
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
