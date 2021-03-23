#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "ExecutionController.h"
#include "Utility.h"

using namespace flow_shop_scheduler;
using nlohmann::json;

json run_test_case(const std::string& inputFilename, const std::string& outputFilename)
{
    json ans;
    std::ifstream ifs(outputFilename);
    ifs >> ans;
    ifs.close();

    ifs.open(inputFilename);
    std::ostringstream oss;

    spdlog::set_level(spdlog::level::trace);

    ExecutionController executionController(ifs, oss);
    executionController.execute();

    json res;
    std::istringstream iss(oss.str());
    iss >> res;

    return json::diff(ans, res);
}

TEST_CASE("Simulator regression tests")
{
    SECTION("All sets are singletons")
    {
        auto diff = run_test_case("input01.json", "output01.json");
        CHECK(diff == "[]"_json);
    }

    SECTION("4 jobs with specified sequence")
    {
        auto diff = run_test_case("input02.json", "output02.json");
        CHECK(diff == "[]"_json);
    }

    SECTION("Multiple operations")
    {
        auto diff = run_test_case("input03.json", "output03.json");
        CHECK(diff == "[]"_json);
    }
}