#ifndef OPERATION_H
#define OPERATION_H

#include "ModelSet.h"
#include "Machine.h"

namespace flow_shop_scheduler {

struct Operation
{
    std::string name;
    int order;
    bool requiresMachine;
    ModelSet<Machine> machines;

    void setDataContext(std::shared_ptr<DataContext>);
    static std::string PathToId();
    static std::string Name(bool plural = false) { return plural ? "operations" : "operation"; }
};

void from_json(const json&, Operation&);
void to_json(json&, const std::pair<const std::size_t, Operation>&);

struct OperationSet : public ModelSet<Operation>
{
    static std::string BasePath();
};

} // namespace flow_shop_scheduler

#endif