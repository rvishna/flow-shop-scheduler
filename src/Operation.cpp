#include "Operation.h"

namespace flow_shop_scheduler {

static const std::string PathToName = "name";
static const std::string PathToOrder = "order";
static const std::string PathToMachines = "machines";

#ifdef USE_CAMEL_CASE
static const std::string PathToRequiresMachine = "requiresMachine";
#else
static const std::string PathToRequiresMachine = "requires_machine";
#endif

static bool DefaultRequiresMachine = true;

std::string Operation::PathToId()
{
    return "id";
}

std::string OperationSet::BasePath()
{
    return "/operations";
}

void Operation::setDataContext(std::shared_ptr<DataContext> dataContext)
{
    for(auto& [_, machine] : machines)
        machine.setDataContext(dataContext);
}

void to_json(json& j, const std::pair<const std::size_t, Operation>& operation)
{
    j[Operation::PathToId()] = operation.first;
    j[PathToName] = operation.second.name;
    j[PathToOrder] = operation.second.order;
    j[PathToRequiresMachine] = operation.second.requiresMachine;
    operation.second.machines.set(j[PathToMachines]);
}

void from_json(const json& j, Operation& operation)
{
    j.at(PathToName).get_to(operation.name);
    j.at(PathToOrder).get_to(operation.order);
    operation.requiresMachine = j.value(PathToRequiresMachine, json{}).get<std::optional<bool>>().value_or(DefaultRequiresMachine);
    if(operation.requiresMachine)
    {
        operation.machines.get(j.at(PathToMachines));
        if(operation.machines.empty())
        {
            std::ostringstream oss;
            oss << "No machine available for scheduling " << Operation::Name() << std::endl
                << std::setw(4) << j;
            throw Exception(oss.str());
        }
    }
}

} // namespace flow_shop_scheduler