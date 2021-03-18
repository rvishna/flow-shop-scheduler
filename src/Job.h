#ifndef JOB_H
#define JOB_H

#include "ModelSet.h"
#include "TimeHorizon.h"

namespace flow_shop_scheduler {

class Job
{
public:
    std::string name;
    std::size_t operationRequirementGroupId;

    int startTimePeriod() const { return startTimePeriod_; }

    void setDataContext(std::shared_ptr<DataContext>);
    static std::string PathToId();
    static std::string Name(bool plural = false) { return plural ? "jobs" : "job"; }

private:
    std::optional<time_type> startTime_;
    int startTimePeriod_;

    friend void from_json(const json&, Job&);
    friend void to_json(json&, const std::pair<const std::size_t, Job>&);
};

void from_json(const json&, Job&);
void to_json(json&, const std::pair<const std::size_t, Job>&);

struct JobSet : public ModelSet<Job>
{
    static std::string BasePath();
};

} // namespace flow_shop_scheduler

#endif