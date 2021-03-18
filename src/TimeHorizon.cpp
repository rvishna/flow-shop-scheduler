#include <regex>

#include "TimeHorizon.h"

using namespace std::chrono_literals;

namespace flow_shop_scheduler {

#ifdef USE_CAMEL_CASE
static const std::string PathToStartTime = "startTime";
static const std::string PathToEndTime = "endTime";
static const std::string PathToTimePeriodDuration = "timePeriodDuration";
#else
static const std::string PathToStartTime = "start_time";
static const std::string PathToEndTime = "end_time";
static const std::string PathToTimePeriodDuration = "time_period_duration";
#endif

static const duration_type DefaultTimePeriodDuration = 24h;

std::string TimeHorizon::BasePath()
{
#ifdef USE_CAMEL_CASE
    return "/timeHorizon";
#else
    return "/time_horizon";
#endif
}

void TimeHorizon::setEndTime(time_type endTime)
{
    auto numberOfTimePeriods = (endTime - startTime) / timePeriodDuration;
    adjustedEndTime_ = startTime + numberOfTimePeriods * timePeriodDuration;
}

time_type TimeHorizon::getPeriodStartTime(int timePeriod) const
{
    assert(timePeriod >= 0 && timePeriod < numberOfTimePeriods());
    return startTime + timePeriod * timePeriodDuration;
}

time_type TimeHorizon::getPeriodEndTime(int timePeriod) const
{
    return getPeriodStartTime(timePeriod) + timePeriodDuration;
}

int TimeHorizon::numberOfTimePeriods() const
{
    return (adjustedEndTime_ - startTime) / timePeriodDuration;
}

double TimeHorizon::daysPerTimePeriod() const
{
    return static_cast<double>(timePeriodDuration.count()) / static_cast<double>(duration_type(24h).count());
}

int TimeHorizon::getLatestPeriodEndsBefore(time_type t) const
{
    return (t - startTime) / timePeriodDuration - 1;
}

int TimeHorizon::getEarliestPeriodStartsAfter(time_type t) const
{
    return numberOfTimePeriods() - (adjustedEndTime_ - t) / timePeriodDuration;
}

int TimeHorizon::getLatestPeriodStartsBefore(time_type t) const
{
    return (t - startTime) / timePeriodDuration;
}

int TimeHorizon::getEarliestPeriodEndsAfter(time_type t) const
{
    return numberOfTimePeriods() - (adjustedEndTime_ - t) / timePeriodDuration - 1;
}

void to_json(json& j, const TimeHorizon& timeHorizon)
{
    j[PathToTimePeriodDuration] = timeHorizon.timePeriodDuration;
    j[PathToStartTime] = timeHorizon.startTime;
    j[PathToEndTime] = timeHorizon.adjustedEndTime_;
}

void from_json(const json& j, TimeHorizon& timeHorizon)
{
    timeHorizon.timePeriodDuration = j.value(PathToTimePeriodDuration, DefaultTimePeriodDuration);
    j.at(PathToStartTime).get_to(timeHorizon.startTime);
    timeHorizon.setEndTime(j[PathToEndTime].get<time_type>());
}

void TimeHorizon::set(json& j) const
{
    to_json(j, *this);
}

void TimeHorizon::get(const json& j, std::shared_ptr<DataContext>)
{
    from_json(j, *this);
}

} // namespace flow_shop_scheduler

namespace nlohmann {

using namespace std::chrono;

void adl_serializer<flow_shop_scheduler::time_type>::from_json(const json& j, flow_shop_scheduler::time_type& t)
{
    std::string s;
    j.get_to(s);

    std::istringstream iss(s);
    iss >> date::parse("%FT%TZ", t);

    if(iss.fail())
    {
        iss.clear();
        iss.str(s);
        iss >> date::parse("%FT%T%z", t);
    }

    if(iss.fail())
        throw detail::type_error::create(399, s + " is not a valid ISO 8601 date time representation");
}

void adl_serializer<flow_shop_scheduler::time_type>::to_json(json& j, const flow_shop_scheduler::time_type& t)
{
    std::ostringstream oss;
    oss << date::format("%FT%TZ", t);
    j = oss.str();
}

void adl_serializer<flow_shop_scheduler::duration_type>::to_json(json& j, const flow_shop_scheduler::duration_type& d)
{
    std::vector<double> us = {31556952000000, 2629746000000, 604800000000, 86400000000, 3600000000, 60000000, 1000000};
    std::string suffixes = "YMWDHMS";

    double ticks = d.count();
    if(ticks == 0)
    {
        j = "PT0S";
        return;
    }

    std::vector<double> v(suffixes.size());
    for(auto i = 0u; i < v.size(); ++i)
    {
        v[i] = (suffixes[i] == 'S' ? ticks / us[i] : std::floor(ticks / us[i]));
        ticks -= v[i] * us[i];
    }

    v.back() = std::floor(1000000 * v.back()) / 1000000.;

    std::ostringstream oss;
    oss << "P";
    for(auto i = 0u; i < v.size(); ++i)
    {
        if(suffixes[i] == 'H' && std::any_of(v.begin() + i, v.end(), [](double d) { return d > 0; }))
            oss << "T";
        if(v[i] > 0)
        {
            if(suffixes[i] == 'S')
                oss << std::fixed << std::setprecision(6) << v[i] << suffixes[i];
            else
                oss << v[i] << suffixes[i];
        }
    }
    j = oss.str();
}

void adl_serializer<flow_shop_scheduler::duration_type>::from_json(const json& j, flow_shop_scheduler::duration_type& d)
{
    static std::unordered_map<std::string, flow_shop_scheduler::duration_type> cache;
    std::string s;
    j.get_to(s);

    if(cache.count(s) != 0)
    {
        d = cache.at(s);
        return;
    }

    std::string decimal = "([-+]?(?:[0-9]+|[0-9]*[.,]?[0-9]*))";
    std::ostringstream pattern;
    pattern << "^(-|\\+)?P";
    std::string suffixes = "YMWDHMS";
    for(auto c : suffixes)
        pattern << (c == 'H' ? "(?:T" : "") << "(?:" << decimal << c << ")?" << (c == 'S' ? ")?$" : "");

    std::regex regex(pattern.str());
    std::smatch m;
    if(!std::regex_match(s, m, regex))
        throw detail::type_error::create(399, s + " is not a valid ISO 8601 duration representation");

    std::vector<double> v;
    std::transform(m.begin() + 2, m.end(), std::back_inserter(v), [](const auto& m_) {
        std::string sub = m_;
        std::replace(sub.begin(), sub.end(), ',', '.');
        std::istringstream iss(sub);
        double c = 0;
        iss >> c;
        return c;
    });
    d = flow_shop_scheduler::duration_type(static_cast<int64_t>(std::round((((v[0] * 12 + v[1]) * 365.2425 / 12 + (7 * v[2] + v[3])) * 86400 + (v[4] * 3600 + v[5] * 60 + v[6])) * 1000000.)));
    if(m[1] == "-")
        d = -d;

    cache[s] = d;
}

} // namespace nlohmann