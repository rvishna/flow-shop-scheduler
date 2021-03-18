#ifndef SIMULATOR_OPTIONS_H
#define SIMULATOR_OPTIONS_H

#include "DataContext.h"

namespace flow_shop_scheduler {

struct SimulatorOptions : public JSONSerializable
{
    static std::string BasePath();

    std::string strategy;

    void set(json&) const override;
    void get(const json&, std::shared_ptr<DataContext> dataContext = nullptr) override;
};

void to_json(json&, const SimulatorOptions&);
void from_json(const json&, SimulatorOptions&);

} // namespace flow_shop_scheduler

#endif