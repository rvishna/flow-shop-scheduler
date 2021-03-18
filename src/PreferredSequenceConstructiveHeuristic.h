#ifndef PREFERRED_SEQUENCE_CONSTRUCTIVE_HEURISTIC_H
#define PREFERRED_SEQUENCE_CONSTRUCTIVE_HEURISTIC_H

#include "Job.h"
#include "ScheduleEvent.h"
#include "SimulationStrategy.h"

namespace flow_shop_scheduler {

namespace detail {

struct OperationMachineSequence
{
    std::vector<std::size_t> machineSequence;

    void setDataContext(std::weak_ptr<DataContext>){}; // no dependencies
    static std::string PathToId();
    static std::string Name(bool plural = false) { return plural ? "sequence of machines for operations" : "sequence of machines for operation"; }
};

void from_json(const json&, std::pair<const std::size_t, OperationMachineSequence>&);
void to_json(json&, const OperationMachineSequence&);

struct PreferredSequence : public JSONSerializable
{
    static std::string BasePath();

    void set(json&) const override;
    void get(const json&, std::shared_ptr<DataContext> dataContext = nullptr) override;

    std::vector<size_t> jobSequence;
    ModelSet<OperationMachineSequence> operationMachineSequence;
};

void from_json(const json&, PreferredSequence&);
void to_json(json&, const PreferredSequence&);

} // namespace detail

class PreferredSequenceConstructiveHeuristicStrategy : public SimulationStrategy
{
public:
    PreferredSequenceConstructiveHeuristicStrategy();
    ~PreferredSequenceConstructiveHeuristicStrategy();

    void setDataContext(std::shared_ptr<DataContext>) override;
    Schedule generate() override;

private:
    class PreferredSequenceConstructiveHeuristicStrategyImpl;
    std::unique_ptr<PreferredSequenceConstructiveHeuristicStrategyImpl> pimpl_;
};

} // namespace flow_shop_scheduler

#endif
