#include "nxe_extension.h"
#include "nxe_instance.h"
#include "navitipc.h"

#include "navitprocessimpl.h"
#include "navitdbus.h"
#include "log.h"
#include "settings.h"
#include "settingtags.h"

using namespace NXE;

extern const char kAscii_nxe_api[];
NXE::NXExtension* g_extension = nullptr;

struct NXE::NXExtensionPrivate {
    std::shared_ptr<NavitProcess> navitProcess;
    std::shared_ptr<NavitIPCInterface> navitIPC;

    NXEInstance* instance = nullptr;
};

common::Extension* CreateExtension()
{
    Settings s;
    const std::string path = s.get<SettingsTags::FileLog>();
    spdlog::rotating_logger_mt("nxe_logger", path, 1048576 * 5, 3, true);
    spdlog::set_level(spdlog::level::debug);
    g_extension = new NXExtension();
    nInfo() << "Plugin loaded. Addr" << static_cast<void*>(g_extension);
    return g_extension;
}

NXExtension::NXExtension()
    : d(new NXExtensionPrivate)
{
    nDebug() << "Creating NXE extension";
    SetExtensionName("nxe");
    SetJavaScriptAPI(kAscii_nxe_api);
}

NXExtension::~NXExtension()
{
}

common::Instance *NXExtension::CreateInstance()
{
    if (d->instance) {
        nFatal() << "This plugin does not support more than one instance yet";
        return nullptr;
    }

    if (!d->navitProcess) {

        // TODO: Properly set Navit path and
        // environment for Navit

        // Use proper DI here?
        d->navitProcess.reset(new NavitProcessImpl);
        d->navitIPC.reset(new NavitDBus);
//        d->navitController.reset (new NavitController { std::make_shared<NavitIPCInterface> { new NavitDBus } });
//        d->navitController.reset(new NavitDBus);
    }

    d->instance =  new NXEInstance(d->navitProcess, d->navitIPC);

    nDebug() << "Created instance. Ptr= " << static_cast<void*>(d->instance);
    return d->instance;
}
