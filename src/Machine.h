#ifndef MACHINE_H
#define MACHINE_H

#include "TimeHorizon.h"

namespace flow_shop_scheduler {

class Machine
{
public:
    std::string name;

    int startTimePeriod() const { return startTimePeriod_; }
    int endTimePeriod() const { return endTimePeriod_; }

    void setDataContext(std::shared_ptr<DataContext>);
    static std::string PathToId();
    static std::string Name(bool plural = false) { return plural ? "machines" : "machine"; }

private:
    std::optional<time_type> startTime_;
    std::optional<time_type> endTime_;

    int startTimePeriod_;
    int endTimePeriod_;

    friend void from_json(const json&, Machine&);
    friend void to_json(json&, const std::pair<const std::size_t, Machine>&);
};

void from_json(const json&, Machine&);
void to_json(json&, const std::pair<const std::size_t, Machine>&);

}

#endif