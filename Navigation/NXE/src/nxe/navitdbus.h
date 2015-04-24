#ifndef NXE_NAVITDBUS_H
#define NXE_NAVITDBUS_H

#include "navitcontroller.h"
#include "inavitipc.h"

#include <fruit/macro.h>

namespace NXE {
class DBusController;

class NavitDBusPrivate;
class NavitDBus : public INavitIPC {
public:
    INJECT(NavitDBus(DBusController& ctrl));
    ~NavitDBus();

    //! This will block until startup signal is received
    virtual void quit() override;
    virtual void moveBy(int x, int y) override;
    virtual void zoomBy(int y) override;
    virtual int zoom() override;
    virtual void render() override;
    virtual int orientation() override;
    virtual void setOrientation(int newOrientation) override;
    virtual void setCenter(double longitude, double latitude) override;
    virtual void setDestination(double longitude, double latitude, const std::string& description) override;
    virtual void setPosition(double longitude, double latitude) override;
    virtual void clearDestination() override;
    virtual void addWaypoint(double longitude, double latitude) override;

    virtual SpeechSignal& speechSignal() override;
    virtual InitializedSignal& initializedSignal() override;

private:
    std::unique_ptr<NavitDBusPrivate> d;
};

} // namespace NXE

#endif // NXE_NAVITDBUS_H
