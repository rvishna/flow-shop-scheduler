#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include <nlohmann/json.hpp>

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
}