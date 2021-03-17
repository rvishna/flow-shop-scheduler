#include "ExecutionController.h"
#include "ServiceController.h"
#include "Utility.h"

using flow_shop_scheduler::ServiceController;

int main(int argc, char* argv[])
{
    std::string port;
#ifdef HTTP_LISTENER_PORT
    port = STRINGIZE(HTTP_LISTENER_PORT);
#endif
    if(port.empty())
        port = "8080";

    signal(SIGINT, flow_shop_scheduler::utility::handleUserInterrupt);

    ServiceController serviceController(port);

    try
    {
        serviceController.start();
        flow_shop_scheduler::utility::waitForUserInterrupt();
    }
    catch(const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }

    serviceController.stop();

    return 0;
}