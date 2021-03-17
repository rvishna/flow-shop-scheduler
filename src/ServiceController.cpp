#include "ServiceController.h"
#include "ExecutionController.h"

namespace flow_shop_scheduler {

ServiceController::ServiceController(const std::string& port)
: port_(port)
, listener_(new web::http::experimental::listener::http_listener("http://0.0.0.0:" + port_))
{
    spdlog::set_pattern("[%Y-%m-%d %T.%f %z] [%^%l%$] %v");
    spdlog::set_level(spdlog::level::info);
    listener_->support(web::http::methods::POST, std::bind(&ServiceController::handle_post, this, std::placeholders::_1));
}

ServiceController::~ServiceController()
{
}

void ServiceController::handle_post(web::http::http_request request)
{
    web::json::value response;
    web::http::status_code code;

    spdlog::info("Received a new request");

    // clang-format off
    request.extract_json(true).then(
        [&response, &code](pplx::task<web::json::value> task) {
            try
            {
                std::istringstream iss(::utility::conversions::to_utf8string(task.get().serialize()));
                std::ostringstream oss;

                ExecutionController executionController(iss, oss);
                executionController.execute();

                spdlog::debug("Received a response");
                spdlog::debug(oss.str());

                response = web::json::value::parse(::utility::conversions::to_string_t(oss.str()));
                code = web::http::status_codes::OK;
            }
            catch(const std::exception& e)
            {
                std::ostringstream oss;
                oss << json{{"error", {{"message", e.what()}}}};
                spdlog::error(e.what());
                response = web::json::value::parse(::utility::conversions::to_string_t(oss.str()));
                code = web::http::status_codes::BadRequest;
            }
        }
    ).wait();
    // clang-format on

    spdlog::info("Sending {} response", code);

    request.reply(code, response);
}

void ServiceController::start()
{
    listener_->open().then([this]() { std::cout << "Starting to listen on port " << port_ << "..." << std::endl; }).wait();
}

void ServiceController::stop()
{
    listener_->close().wait();
}

} // namespace flow_shop_scheduler