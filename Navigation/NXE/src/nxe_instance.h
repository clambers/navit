#ifndef NXE_NXEINSTANCE_H
#define NXE_NXEINSTANCE_H

#include "common/extension.h"
#include <memory>

namespace NXE {
class NavitProcess;
class NavitController;

class NXEInstancePrivate;
class NXEInstance : public common::Instance
{
public:
    NXEInstance(std::weak_ptr<NavitProcess> navitProcess, std::weak_ptr<NavitController> controller);
    ~NXEInstance();

    void HandleMessage(const char *msg) override;
private:
    std::unique_ptr<NXEInstancePrivate> d_ptr;
};

} // namespace NXE

#endif // NXE_NXEINSTANCE_H
