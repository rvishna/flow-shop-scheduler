#ifndef FLOW_SHOP_SCHEDULER_UTILITY_H
#define FLOW_SHOP_SCHEDULER_UTILITY_H

#define STR(x) #x
#define STRINGIZE(x) STR(x)

#include <signal.h>

#include <iostream>

namespace flow_shop_scheduler {

namespace utility {

void handleUserInterrupt(int signal);
void waitForUserInterrupt();

} // namespace utility

} // namespace flow_shop_scheduler

#endif