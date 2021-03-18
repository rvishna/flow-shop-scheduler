#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include "Operation.h"

using namespace std::chrono_literals;
using namespace flow_shop_scheduler;
using nlohmann::json;

struct MockTimeHorizon : public TimeHorizon
{
    int getEarliestPeriodStartsAfter(time_type) const override { return 1; }
    int getLatestPeriodEndsBefore(time_type) const override { return 1; }
    int numberOfTimePeriods() const override { return 1; }
};

SCENARIO("Operations can be deserialized from JSON")
{
    GIVEN("A JSON array of objects and a data context")
    {
        auto dataContext = std::make_shared<DataContext>();
        dataContext->add(std::make_unique<MockTimeHorizon>());

        auto j = R"(
            [
                {
                    "id": 1,
                    "name": "Drilling",
                    "order": 1,
                    "machines": [
                        {
                            "id": 1,
                            "name": "Rig 1",
                            "start_time": "2021-01-01T00:00:00Z"
                        },
                        {
                            "id": 2,
                            "name": "Rig 2",
                            "end_time": "2023-06-01T00:00:00Z"
                        }
                    ]
                },
                {
                    "id": 2,
                    "name": "Fracking",
                    "order": 2,
                    "machines": [
                        {
                            "id": 3,
                            "name": "Crew 1",
                            "start_time": "2021-01-01T00:00:00Z"
                        }
                    ]
                },
                {
                    "id": 3,
                    "name": "Production Lag",
                    "order": 3,
                    "requires_machine": false
                },
                {
                    "id": 4,
                    "name": "Producing",
                    "order": 4,
                    "requires_machine": false
                }
            ]
        )"_json;

        WHEN("it is deserialized to a set of operations")
        {
            OperationSet operations;
            operations.get(j, dataContext);

            THEN("the deserialized set of operations matches the expected result")
            {
                CHECK(operations.size() == 4u);

                CHECK(operations[1].name == "Drilling");
                CHECK(operations[1].order == 1);

                CHECK(operations[1].machines[1].name == "Rig 1");
                CHECK(operations[1].machines[1].startTimePeriod() == 1);
                CHECK(operations[1].machines[1].endTimePeriod() == 0);

                CHECK(operations[1].machines[2].name == "Rig 2");
                CHECK(operations[1].machines[2].startTimePeriod() == 0);
                CHECK(operations[1].machines[2].endTimePeriod() == 1);

                CHECK(operations[2].name == "Fracking");
                CHECK(operations[2].order == 2);

                CHECK(operations[2].machines[3].name == "Crew 1");
                CHECK(operations[2].machines[3].startTimePeriod() == 1);
                CHECK(operations[2].machines[3].endTimePeriod() == 0);

                CHECK(operations[3].name == "Production Lag");
                CHECK(operations[3].order == 3);

                CHECK(operations[4].name == "Producing");
                CHECK(operations[4].order == 4);
            }
        }
    }
}