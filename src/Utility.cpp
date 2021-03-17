#include <condition_variable>
#include <mutex>
#include <thread>

#include "Utility.h"

namespace flow_shop_scheduler {

namespace utility {

static std::condition_variable _condition;
static std::mutex _mutex;

void handleUserInterrupt(int signal)
{
    if(signal == SIGINT)
        _condition.notify_one();
}

void waitForUserInterrupt()
{
    std::unique_lock<std::mutex> lock{_mutex};
    _condition.wait(lock);
    lock.unlock();
}

} // namespace utility

} // namespace flow_shop_scheduler