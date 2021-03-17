#ifndef SERVICE_CONTROLLER_H
#define SERVICE_CONTROLLER_H

#include <iomanip>

#include <cpprest/http_listener.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "Utility.h"

namespace flow_shop_scheduler {

using nlohmann::json;

class ServiceController
{
public:
    ServiceController(const std::string& port = "8080");
    virtual ~ServiceController();

    void start();
    void stop();

private:
    std::string port_;
    std::unique_ptr<web::http::experimental::listener::http_listener> listener_;

    void handle_post(web::http::http_request request);
};

} // namespace flow_shop_scheduler

#endif