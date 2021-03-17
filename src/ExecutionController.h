#ifndef EXECUTION_CONTROLLER_H
#define EXECUTION_CONTROLLER_H

#include <memory>

namespace flow_shop_scheduler {

class ExecutionController
{
public:
    ExecutionController(std::istream& is, std::ostream& os);
    ~ExecutionController();
    void execute() const;

private:
    class ExecutionControllerImpl;
    std::unique_ptr<ExecutionControllerImpl> pimpl_;
};

} // namespace flow_shop_scheduler

#endif
