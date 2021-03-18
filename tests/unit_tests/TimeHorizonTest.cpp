#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include "TimeHorizon.h"

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace date;
using namespace flow_shop_scheduler;
using nlohmann::json;

TEST_CASE("Time type can be deserialized")
{
    SECTION("from valid ISO 8601 date time representations")
    {
        SECTION("ending with Z")
        {
            CHECK(json("2019-01-03T06:30:05Z").get<time_type>() == sys_days{January / 3 / 2019} + 6h + 30min + 5s);
        }
        SECTION("ending with an offset")
        {
            CHECK(json("1923-01-05T16:57:08-0630").get<time_type>() == sys_days{January / 5 / 1923} + 23h + 27min + 8s);
            CHECK(json("2060-08-31T00:03:00+1000").get<time_type>() == sys_days{August / 30 / 2060} + 14h + 3min);
            CHECK(json("2040-02-28T23:37:18-1030").get<time_type>() == sys_days{February / 29 / 2040} + 10h + 7min + 18s);
        }
        SECTION("with fractional seconds")
        {
            CHECK(json("2018-05-04T03:40:55.186Z").get<time_type>() == sys_days{May / 4 / 2018} + 3h + 40min + 55s + 186ms);
            CHECK(json("2023-11-07T12:12:40.123456+0030").get<time_type>() == sys_days{November / 7 / 2023} + 11h + 42min + 40s + 123456us);
        }
    }
}

TEST_CASE("Time type can be serialized")
{
    SECTION("with a date time")
    {
        time_type t = sys_days{October / 24 / 2011} + 19h + 1243us;
        json j = t;
        CHECK(j.get<std::string>() == "2011-10-24T19:00:00.001243Z");
    }
    SECTION("with a date only")
    {
        time_type t = sys_days{December / 31 / 1999};
        json j = t;
        CHECK(j.get<std::string>() == "1999-12-31T00:00:00.000000Z");
    }
}

TEST_CASE("Duration type can be deserialized")
{
    SECTION("for the test cases borrowed from moment.js source code")
    {
        CHECK(json("P1Y2M3DT4H5M6S").get<duration_type>() == years(1) + months(2) + days(3) + hours(4) + minutes(5) + seconds(6));
        CHECK(json("P3W3D").get<duration_type>() == days(24));
        CHECK(json("P1M").get<duration_type>() == months(1));
        CHECK(json("PT1M").get<duration_type>() == minutes(1));
        CHECK(json("P1MT2H").get<duration_type>() == months(1) + hours(2));
        CHECK(json("-P60D").get<duration_type>() == -days(60));
        CHECK(json("+P60D").get<duration_type>() == days(60));
        CHECK(json("PT0.5S").get<duration_type>() == milliseconds(500));
        CHECK(json("PT0,5S").get<duration_type>() == milliseconds(500));
    }
}

TEST_CASE("Duration type can be serialized")
{
    CHECK(json(duration_type(1)).get<std::string>() == "PT0.000001S");
    CHECK(json(duration_type(120h)).get<std::string>() == "P5D");
}

TEST_CASE("Time horizon utility functions work")
{
    GIVEN("A time horizon object with time period duration set to 24 hours")
    {
        TimeHorizon timeHorizon;
        timeHorizon.timePeriodDuration = 24h;
        timeHorizon.startTime = sys_days{January / 1 / 2019};
        timeHorizon.setEndTime(sys_days{January / 1 / 2020});

        WHEN("The number of time periods is computed")
        {
            THEN("The result matches the expected answer")
            {
                CHECK(timeHorizon.numberOfTimePeriods() == 365);
            }
        }

        WHEN("The start time for a particular time period is computed")
        {
            THEN("The result matches the expected answer")
            {
                CHECK(timeHorizon.getPeriodStartTime(0) == sys_days{January / 1 / 2019});
                CHECK(timeHorizon.getPeriodStartTime(100) == sys_days{April / 11 / 2019});
                CHECK(timeHorizon.getPeriodStartTime(364) == sys_days{December / 31 / 2019});
            }
        }

        WHEN("The end time for a particular time period is computed")
        {
            THEN("The result matches the expected answer")
            {
                CHECK(timeHorizon.getPeriodEndTime(0) == sys_days{January / 2 / 2019});
                CHECK(timeHorizon.getPeriodEndTime(364) == sys_days{January / 1 / 2020});
            }
        }

        WHEN("The earliest period that starts after a given time is computed")
        {
            THEN("The result matches the expected answer")
            {
                CHECK(timeHorizon.getEarliestPeriodStartsAfter(sys_days{January / 21 / 2019}) == 20);
                CHECK(timeHorizon.getEarliestPeriodStartsAfter(sys_days{January / 10 / 2019} + 13h + 30min) == 10);
            }
        }

        WHEN("The latest period that ends before a given time is computed")
        {
            THEN("The result matches the expected answer")
            {
                CHECK(timeHorizon.getLatestPeriodEndsBefore(sys_days{January / 21 / 2019}) == 19);
                CHECK(timeHorizon.getLatestPeriodEndsBefore(sys_days{January / 10 / 2019} + 13h + 30min) == 8);
            }
        }
    }

    GIVEN("A time horizon object with time period duration set to 120 hours")
    {
        TimeHorizon timeHorizon;
        timeHorizon.timePeriodDuration = 120h;
        timeHorizon.startTime = sys_days{January / 1 / 2016};
        timeHorizon.setEndTime(sys_days{January / 1 / 2021});

        WHEN("The number of time periods is computed")
        {
            THEN("The result matches the expected answer")
            {
                CHECK(timeHorizon.numberOfTimePeriods() == 365);
            }
        }

        WHEN("The start time for a particular time period is computed")
        {
            THEN("The result matches the expected answer")
            {
                CHECK(timeHorizon.getPeriodStartTime(0) == sys_days{January / 1 / 2016});
                CHECK(timeHorizon.getPeriodStartTime(89) == sys_days{March / 21 / 2017});
                CHECK(timeHorizon.getPeriodStartTime(364) == sys_days{December / 25 / 2020});
            }
        }

        WHEN("The end time for a particular time period is computed")
        {
            THEN("The result matches the expected answer")
            {
                CHECK(timeHorizon.getPeriodEndTime(0) == sys_days{January / 6 / 2016});
                CHECK(timeHorizon.getPeriodEndTime(364) == sys_days{December / 30 / 2020});
            }
        }

        WHEN("The earliest period that starts after a given time is computed")
        {
            THEN("The result matches the expected answer")
            {
                CHECK(timeHorizon.getEarliestPeriodStartsAfter(sys_days{January / 7 / 2016}) == 2);
                CHECK(timeHorizon.getEarliestPeriodStartsAfter(sys_days{October / 18 / 2017}) == 132);
                CHECK(timeHorizon.getEarliestPeriodStartsAfter(sys_days{January / 11 / 2016}) == 2);
            }
        }

        WHEN("The latest period that ends before a given time is computed")
        {
            THEN("The result matches the expected answer")
            {
                CHECK(timeHorizon.getLatestPeriodEndsBefore(sys_days{October / 18 / 2017}) == 130);
                CHECK(timeHorizon.getLatestPeriodEndsBefore(sys_days{January / 11 / 2016}) == 1);
            }
        }
    }
}