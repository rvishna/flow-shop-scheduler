#include <iostream>

#include <spdlog/spdlog.h>

#include "Exception.h"
#include "ExecutionController.h"

using flow_shop_scheduler::Exception;
using flow_shop_scheduler::ExecutionController;

int main(int argc, char* argv[])
{
    try
    {
        spdlog::set_level(spdlog::level::off);

        ExecutionController executionController(std::cin, std::cout);
        executionController.execute();
    }
    catch(const Exception& e)
    {
        std::cout << "Flow Shop Scheduler Exception: " << e.what() << std::endl;
    }
    catch(const std::exception& e)
    {
        std::cout << "Standard Library Exception: " << e.what() << std::endl;
    }

    return 0;
}
