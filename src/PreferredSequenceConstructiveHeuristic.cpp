#include <unordered_map>

#include "Operation.h"
#include "OperationRequirementGroup.h"
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

    int numberOfTimePeriods_;
    int numberOfOperations_;
    int numberOfJobs_;
    int numberOfMachines_;

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
    std::unordered_map<std::size_t, std::unordered_map<std::size_t, const OperationRequirementGroup*>> jobOperationRequirements_;

    bool isMachineAvailable(std::size_t machineId, int startTimePeriod, int numberOfTimePeriods = 1) const;
    void addScheduleEvent(Schedule& schedule, const ScheduleEvent& scheduleEvent, bool updateCache = true);
    bool isFeasible(Schedule& schedule, const ScheduleEvent& scheduleEvent) const;
    void releaseMachine(const Schedule& schedule);
};

void PreferredSequenceConstructiveHeuristicStrategy::PreferredSequenceConstructiveHeuristicStrategyImpl::setPreferredSequence(const detail::PreferredSequence& preferredSequence)
{
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

    const auto& operations = dataContext_->get<OperationSet>();
    numberOfOperations_ = static_cast<int>(operations.size());

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

    numberOfJobs_ = 0;

    const auto& jobs = dataContext_->get<JobSet>();
    numberOfJobs_ = static_cast<int>(jobs.size());
    std::transform(jobs.begin(), jobs.end(), std::back_inserter(jobIds_), [](const auto& job) { return job.first; });

    for(auto [machineId, machine] : machines_)
    {
        auto& machineAvailability = machineAvailability_[machineId];
        machineAvailability.assign(numberOfTimePeriods_, false);
        for(int timePeriod = machine->startTimePeriod(); timePeriod <= machine->endTimePeriod(); ++timePeriod)
            machineAvailability[timePeriod] = true;
    }

    const auto& operationRequirementGroups = dataContext_->get<OperationRequirementGroupSet>();

    for(auto operationId : operationIds_)
    {
        for(auto [jobId, job_] : jobs_)
        {
            jobOperationInfo_[jobId][operationId].isScheduled = false;

            const auto& job = *job_;

            auto operationRequirementGroupIt = operationRequirementGroups.find(job.operationRequirementGroupId);
            if(operationRequirementGroupIt == operationRequirementGroups.end())
            {
                std::ostringstream oss;
                oss << "Missing operation requirement group for job" << std::endl
                    << std::setw(4) << json(std::pair<const std::size_t, Job>({jobId, job}));
                throw Exception(oss.str());
            }

            jobOperationRequirements_[jobId][operationId] = &operationRequirementGroupIt->second;
        }
    }

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

bool PreferredSequenceConstructiveHeuristicStrategy::PreferredSequenceConstructiveHeuristicStrategyImpl::isFeasible(Schedule&, const ScheduleEvent& scheduleEvent) const
{
    // Machine is busy or not available.
    if(scheduleEvent.machineId.has_value() && !isMachineAvailable(scheduleEvent.machineId.value(), scheduleEvent.startTimePeriod(), scheduleEvent.numberOfTimePeriods()))
    {
        return false;
    }

    // Job cannot be started yet.
    if(scheduleEvent.startTimePeriod() < jobs_.at(scheduleEvent.jobId)->startTimePeriod())
    {
        return false;
    }

    return true;
}

void PreferredSequenceConstructiveHeuristicStrategy::PreferredSequenceConstructiveHeuristicStrategyImpl::releaseMachine(const Schedule& schedule)
{
    for(auto scheduleEvent : schedule)
        std::fill_n(machineAvailability_[scheduleEvent.machineId.value()].begin() + scheduleEvent.startTimePeriod(), scheduleEvent.numberOfTimePeriods(), true);
}

Schedule PreferredSequenceConstructiveHeuristicStrategy::PreferredSequenceConstructiveHeuristicStrategyImpl::generate()
{
    Schedule schedule;
    const auto& fixedSchedule = dataContext_->get<Schedule>();
    if(!fixedSchedule.empty())
        std::for_each(fixedSchedule.begin(), fixedSchedule.end(), [this, &schedule](const auto& scheduleEvent) { addScheduleEvent(schedule, scheduleEvent); });

    const auto& operations = dataContext_->get<OperationSet>();

    for(auto operationIndex = 0u; operationIndex < operationIds_.size(); ++operationIndex)
    {
        auto operationId = operationIds_[operationIndex];
        const auto& operation = operations[operationId];

        if(!operation.requiresMachine)
        {
            for(auto jobId : jobIds_)
            {
                unsigned int numberOfTimePeriods = jobOperationRequirements_.at(jobId).at(operationId)->numberOfTimePeriods();
                if(operationIndex == 0)
                {
                    addScheduleEvent(schedule, ScheduleEvent(dataContext_, operationIds_[0], jobId, std::nullopt, 0, numberOfTimePeriods));
                }
                else
                {
                    const auto& previousOperationWellInfo = jobOperationInfo_.at(jobId).at(operationIds_[operationIndex - 1]);
                    if(!previousOperationWellInfo.isScheduled || previousOperationWellInfo.startTimePeriod + previousOperationWellInfo.numberOfTimePeriods >= static_cast<unsigned int>(numberOfTimePeriods_))
                        continue;
                    addScheduleEvent(schedule, ScheduleEvent(dataContext_, operationIds_[operationIndex], jobId, std::nullopt, previousOperationWellInfo.startTimePeriod + previousOperationWellInfo.numberOfTimePeriods, numberOfTimePeriods));
                }
            }
        }
        else
        {
            for(int timePeriod = 0; timePeriod < numberOfTimePeriods_; ++timePeriod)
            {
                for(auto machineId : operationMachineIds_.at(operationId))
                {
                    if(!isMachineAvailable(machineId, timePeriod))
                    {
                        continue;
                    }

                    for(auto jobId : jobIds_)
                    {
                        const auto& currentJobOperationInfo = jobOperationInfo_.at(jobId).at(operationIds_[operationIndex]);
                        if(currentJobOperationInfo.isScheduled)
                        {
                            continue;
                        }

                        if(operationIndex > 0)
                        {
                            const auto& previousJobOperationInfo = jobOperationInfo_.at(jobId).at(operationIds_[operationIndex - 1]);
                            if(!previousJobOperationInfo.isScheduled || static_cast<unsigned int>(timePeriod) < previousJobOperationInfo.startTimePeriod + previousJobOperationInfo.numberOfTimePeriods)
                            {
                                break;
                            }
                        }

                        unsigned int numberOfTimePeriods = jobOperationRequirements_.at(jobId).at(operationId)->numberOfTimePeriods();

                        ScheduleEvent scheduleEvent(dataContext_, operationId, jobId, machineId, timePeriod, numberOfTimePeriods);
                        if(!isFeasible(schedule, scheduleEvent))
                        {
                            break;
                        }
                        addScheduleEvent(schedule, scheduleEvent);
                    }
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
