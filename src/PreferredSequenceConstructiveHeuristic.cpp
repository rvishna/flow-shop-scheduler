#include <unordered_map>

#include <spdlog/spdlog.h>

#include "Operation.h"
#include "PreferredSequenceConstructiveHeuristic.h"

namespace flow_shop_scheduler {

namespace detail {

#ifdef USE_CAMEL_CASE
static const std::string PathToJobSequence = "jobSequence";
static const std::string PathToMachineSequence = "machineSequence";
static const std::string PathToOperationMachineSequence = "operationMachineSequence";
#else
static const std::string PathToJobSequence = "job_sequence";
static const std::string PathToMachineSequence = "machine_sequence";
static const std::string PathToOperationMachineSequence = "operation_machine_sequence";
#endif

std::string PreferredSequence::BasePath()
{
#ifdef USE_CAMEL_CASE
    return "/preferredSequence";
#else
    return "/preferred_sequence";
#endif
}

std::string OperationMachineSequence::PathToId()
{
    return "operation";
}

void PreferredSequence::set(json& j) const
{
    to_json(j, *this);
}

void PreferredSequence::get(const json& j, std::shared_ptr<DataContext>)
{
    from_json(j, *this);
}

void from_json(const json& j, OperationMachineSequence& operationMachineSequence)
{
    j.value(PathToMachineSequence, json::array()).get_to(operationMachineSequence.machineSequence);
}

void to_json(json& j, const std::pair<const std::size_t, OperationMachineSequence>& operationMachineSequence)
{
    j[OperationMachineSequence::PathToId()] = operationMachineSequence.first;
    j[PathToMachineSequence] = operationMachineSequence.second.machineSequence;
}

void from_json(const json& j, PreferredSequence& preferredSequence)
{
    j.value(PathToJobSequence, json::array()).get_to(preferredSequence.jobSequence);
    preferredSequence.operationMachineSequence.get(j.value(PathToOperationMachineSequence, json::array()));
}

void to_json(json& j, const PreferredSequence& preferredSequence)
{
    j[PathToJobSequence] = preferredSequence.jobSequence;
    preferredSequence.operationMachineSequence.set(j[PathToOperationMachineSequence]);
}

} // namespace detail

class PreferredSequenceConstructiveHeuristicStrategy::PreferredSequenceConstructiveHeuristicStrategyImpl
{
public:
    void setDataContext(std::shared_ptr<DataContext> dataContext);
    Schedule generate();
    void setPreferredSequence(const detail::PreferredSequence&);

private:
    std::shared_ptr<DataContext> dataContext_;

    unsigned int numberOfTimePeriods_;
    unsigned int numberOfOperations_;
    unsigned int numberOfJobs_;
    unsigned int numberOfMachines_;

    std::unordered_map<std::size_t, const Job*> jobs_;
    std::unordered_map<std::size_t, const Machine*> machines_;

    std::vector<std::size_t> operationIds_;
    std::vector<std::size_t> jobIds_;
    std::unordered_map<std::size_t, std::vector<std::size_t>> operationMachineIds_;

    struct OperationJobInfo
    {
        bool isScheduled;
        int startTimePeriod;
        unsigned int numberOfTimePeriods;
        std::optional<std::size_t> assignedMachineId;
    };

    std::unordered_map<std::size_t, std::unordered_map<std::size_t, OperationJobInfo>> jobOperationInfo_;
    std::unordered_map<std::size_t, std::vector<bool>> machineAvailability_;
    std::unordered_map<std::size_t, std::unordered_map<std::size_t, const OperationRequirement*>> jobOperationRequirements_;

    bool isMachineAvailable(std::size_t machineId, int startTimePeriod, int numberOfTimePeriods = 1) const;
    void addScheduleEvent(Schedule& schedule, const ScheduleEvent& scheduleEvent, bool updateCache = true);
};

void PreferredSequenceConstructiveHeuristicStrategy::PreferredSequenceConstructiveHeuristicStrategyImpl::setPreferredSequence(const detail::PreferredSequence& preferredSequence)
{
    spdlog::debug("Setting preferred sequence of machines");
    for(auto operationId : operationIds_)
    {
        if(!preferredSequence.operationMachineSequence.count(operationId))
            continue;

        std::unordered_map<std::size_t, int> machineIdToPriorityMap;
        const auto& preferredMachineSequence = preferredSequence.operationMachineSequence[operationId].machineSequence;
        std::transform(preferredMachineSequence.begin(), preferredMachineSequence.end(), std::inserter(machineIdToPriorityMap, machineIdToPriorityMap.end()), [machinePriority = 0](std::size_t machineId) mutable { return std::make_pair(machineId, machinePriority++); });

        auto& operationMachineIds = operationMachineIds_.at(operationId);
        std::sort(operationMachineIds.begin(), operationMachineIds.end(), [&machineIdToPriorityMap](const auto& lhs, const auto& rhs) {
            if(machineIdToPriorityMap.count(lhs) && machineIdToPriorityMap.count(rhs))
                return machineIdToPriorityMap.at(lhs) < machineIdToPriorityMap.at(rhs);
            else if(machineIdToPriorityMap.count(lhs))
                return true;
            return false;
        });
    }

    spdlog::debug("Setting preferred sequence of jobs");
    std::unordered_map<std::size_t, int> jobIdToPriorityMap;
    std::transform(preferredSequence.jobSequence.begin(), preferredSequence.jobSequence.end(), std::inserter(jobIdToPriorityMap, jobIdToPriorityMap.end()), [jobPriority = 0](std::size_t jobId) mutable { return std::make_pair(jobId, jobPriority++); });
    std::sort(jobIds_.begin(), jobIds_.end(), [&jobIdToPriorityMap](const auto& lhs, const auto& rhs) {
        if(jobIdToPriorityMap.count(lhs) && jobIdToPriorityMap.count(rhs))
            return jobIdToPriorityMap.at(lhs) < jobIdToPriorityMap.at(rhs);
        else if(jobIdToPriorityMap.count(lhs))
            return true;
        return false;
    });
}

void PreferredSequenceConstructiveHeuristicStrategy::PreferredSequenceConstructiveHeuristicStrategyImpl::setDataContext(std::shared_ptr<DataContext> dataContext)
{
    dataContext_ = dataContext;

    dataContext_->add(std::make_unique<detail::PreferredSequence>());
    dataContext_->parse(std::cin);

    numberOfTimePeriods_ = dataContext_->get<TimeHorizon>().numberOfTimePeriods();
    spdlog::debug("Number of time periods is {}", numberOfTimePeriods_);

    const auto& operations = dataContext_->get<OperationSet>();
    numberOfOperations_ = static_cast<int>(operations.size());
    spdlog::debug("Number of operations is {}", numberOfOperations_);

    // Order operations according to the 'order' field
    std::transform(operations.begin(), operations.end(), std::back_inserter(operationIds_), [](const auto& operation) { return operation.first; });
    std::sort(operationIds_.begin(), operationIds_.end(), [&operations](const auto& lhs, const auto& rhs) { return operations[lhs].order < operations[rhs].order; });

    numberOfMachines_ = 0;

    for(const auto& [operationId, operation] : operations)
    {
        if(operation.requiresMachine)
        {
            for(const auto& [machineId, machine] : operation.machines)
            {
                machines_[machineId] = &machine;
                operationMachineIds_[operationId].push_back(machineId);
                numberOfMachines_++;
            }
        }
    }

    spdlog::debug("Number of machines is {}", numberOfMachines_);

    numberOfJobs_ = 0;

    for(const auto& [jobId, job] : dataContext_->get<JobSet>())
    {
        jobIds_.push_back(jobId);
        jobs_[jobId] = &job;
        numberOfJobs_++;
    }

    spdlog::debug("Number of jobs is {}", numberOfJobs_);

    for(auto [machineId, machine] : machines_)
    {
        auto& machineAvailability = machineAvailability_[machineId];
        machineAvailability.assign(numberOfTimePeriods_, false);
        for(int timePeriod = machine->startTimePeriod(); timePeriod <= machine->endTimePeriod(); ++timePeriod)
            machineAvailability[timePeriod] = true;
    }

    for(auto [jobId, job_] : jobs_)
    {
        for(const auto& operationRequirement : job_->operationRequirements)
        {
            auto operationId = operationRequirement.operationId;
            jobOperationRequirements_[jobId][operationId] = &operationRequirement;
            spdlog::trace("Found operation requirement\n{}\nfor operation \"{}\", job\n{}",
                          json(operationRequirement).dump(4),
                          operations[operationId].name,
                          json(std::pair<const std::size_t, Job>({jobId, *job_})).dump(4));
        }

        for(auto operationId : operationIds_)
        {
            if(!jobOperationRequirements_[jobId].count(operationId))
            {
                std::ostringstream oss;
                oss << "Missing operation requirement for operation \"" << operations[operationId].name
                    << "\", job " << std::endl
                    << std::setw(4) << json(std::pair<const std::size_t, Job>({jobId, *job_}));
                throw Exception(oss.str());
            }
        }
    }

    for(auto operationId : operationIds_)
        for(auto [jobId, job_] : jobs_)
            jobOperationInfo_[jobId][operationId].isScheduled = false;

    const auto& preferredSequence = dataContext_->get<detail::PreferredSequence>();
    setPreferredSequence(preferredSequence);
}

bool PreferredSequenceConstructiveHeuristicStrategy::PreferredSequenceConstructiveHeuristicStrategyImpl::isMachineAvailable(std::size_t machineId, int startTimePeriod, int numberOfTimePeriods) const
{
    if(startTimePeriod < machines_.at(machineId)->startTimePeriod() || startTimePeriod + numberOfTimePeriods - 1 > machines_.at(machineId)->endTimePeriod())
        return false;
    return std::all_of(machineAvailability_.at(machineId).begin() + startTimePeriod, machineAvailability_.at(machineId).begin() + startTimePeriod + numberOfTimePeriods, [](const auto& b) { return b; });
}

void PreferredSequenceConstructiveHeuristicStrategy::PreferredSequenceConstructiveHeuristicStrategyImpl::addScheduleEvent(Schedule& schedule, const ScheduleEvent& scheduleEvent, bool updateCache)
{
    spdlog::trace("Adding schedule event\n{}", json(scheduleEvent).dump(4));
    schedule.push_back(scheduleEvent);

    if(scheduleEvent.machineId.has_value())
        std::fill_n(machineAvailability_[scheduleEvent.machineId.value()].begin() + scheduleEvent.startTimePeriod(), scheduleEvent.numberOfTimePeriods(), false);

    if(updateCache)
    {
        auto& info = jobOperationInfo_[scheduleEvent.jobId][scheduleEvent.operationId];
        info.startTimePeriod = scheduleEvent.startTimePeriod();
        info.numberOfTimePeriods = scheduleEvent.numberOfTimePeriods();
        info.isScheduled = true;
        info.assignedMachineId = scheduleEvent.machineId;
    }
}

Schedule PreferredSequenceConstructiveHeuristicStrategy::PreferredSequenceConstructiveHeuristicStrategyImpl::generate()
{
    Schedule schedule;
    const auto& fixedSchedule = dataContext_->get<Schedule>();
    if(!fixedSchedule.empty())
        std::for_each(fixedSchedule.begin(), fixedSchedule.end(), [this, &schedule](const auto& scheduleEvent) { addScheduleEvent(schedule, scheduleEvent); });

    const auto& operations = dataContext_->get<OperationSet>();

    for(auto jobId : jobIds_)
    {
        for(auto operationIndex = 0u; operationIndex < operationIds_.size(); ++operationIndex)
        {
            auto operationId = operationIds_[operationIndex];
            const auto& operation = operations[operationId];

            const auto& currentJobOperationInfo = jobOperationInfo_.at(jobId).at(operationIds_[operationIndex]);
            if(currentJobOperationInfo.isScheduled)
                continue;

            int startTimePeriod = jobs_.at(jobId)->startTimePeriod();

            if(operationIndex > 0)
            {
                const auto& previousJobOperationInfo = jobOperationInfo_.at(jobId).at(operationIds_[operationIndex - 1]);
                if(previousJobOperationInfo.isScheduled)
                    startTimePeriod = previousJobOperationInfo.startTimePeriod + previousJobOperationInfo.numberOfTimePeriods;
            }

            unsigned int numberOfTimePeriods = jobOperationRequirements_.at(jobId).at(operationId)->numberOfTimePeriods();

            spdlog::trace("Trying to schedule operation \"{}\" for job \"{}\" from time period {} for {} time periods",
                          operation.name, jobs_.at(jobId)->name, startTimePeriod, numberOfTimePeriods);

            if(!operation.requiresMachine)
            {
                addScheduleEvent(schedule, ScheduleEvent(dataContext_, operationId, jobId, std::nullopt, startTimePeriod, numberOfTimePeriods));
            }
            else
            {
                for(int timePeriod = startTimePeriod; timePeriod + numberOfTimePeriods < numberOfTimePeriods_; ++timePeriod)
                {
                    for(auto machineId : operationMachineIds_.at(operationId))
                    {
                        if(isMachineAvailable(machineId, timePeriod, numberOfTimePeriods))
                        {
                            addScheduleEvent(schedule, ScheduleEvent(dataContext_, operationId, jobId, machineId, timePeriod, numberOfTimePeriods));
                            break;
                        }
                    }

                    if(currentJobOperationInfo.isScheduled)
                        break;
                }
            }
        }
    }
    return schedule;
}

PreferredSequenceConstructiveHeuristicStrategy::PreferredSequenceConstructiveHeuristicStrategy()
: pimpl_(new PreferredSequenceConstructiveHeuristicStrategyImpl)
{
}

PreferredSequenceConstructiveHeuristicStrategy::~PreferredSequenceConstructiveHeuristicStrategy() = default;

Schedule PreferredSequenceConstructiveHeuristicStrategy::generate()
{
    return pimpl_->generate();
}

void PreferredSequenceConstructiveHeuristicStrategy::setDataContext(std::shared_ptr<DataContext> dataContext)
{
    pimpl_->setDataContext(dataContext);
}

} // namespace flow_shop_scheduler
