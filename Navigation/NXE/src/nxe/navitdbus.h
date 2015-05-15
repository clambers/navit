#ifndef NXE_NAVITDBUS_H
#define NXE_NAVITDBUS_H

#include "inavitipc.h"


namespace NXE {
class DBusController;

class NavitDBusPrivate;
class NavitDBus : public INavitIPC {
public:
    NavitDBus(DBusController& ctrl);
    ~NavitDBus();

    //! This will block until startup signal is received
    virtual void quit() override;
    virtual void moveBy(int x, int y) override;
    virtual void zoomBy(int y) override;
    virtual void setZoom(int newZoom) override;
    virtual int zoom() override;
    virtual void render() override;
    virtual void resize(int x, int y) override;
    virtual int orientation() override;
    virtual void setOrientation(int newOrientation) override;
    virtual void setCenter(double longitude, double latitude) override;
    virtual void setDestination(double longitude, double latitude, const std::string& description) override;
    virtual bool isNavigationRunning() override;
    virtual void setPosition(double longitude, double latitude) override;
    virtual void setPositionByInt(int x, int y) override;
    virtual void clearDestination() override;
    virtual void addWaypoint(double longitude, double latitude) override;
    virtual void setScheme(const std::string& scheme) override;
    virtual void setPitch(std::uint16_t newPitchValue) override;

    virtual void searchPOIs(double longitude, double latitude, int distance) override;
    virtual NXE::Position currentCenter() override;

    virtual void startSearch() override;
    virtual SearchResults search(SearchType type, const std::string& searchString) override;
    virtual void selectSearchResult(SearchType type, std::int32_t id) override;
    virtual void finishSearch() override;
    virtual std::int32_t distance() override;
    virtual std::int32_t eta() override;

    virtual SpeechSignalType& speechSignal() override;
    virtual PointClickedSignalType& pointClickedSignal() override;
    virtual InitializedSignalType& initializedSignal() override;
    virtual RoutingSignalType& routingSignal() override;

private:
    std::unique_ptr<NavitDBusPrivate> d;
};

} // namespace NXE

#endif // NXE_NAVITDBUS_H
