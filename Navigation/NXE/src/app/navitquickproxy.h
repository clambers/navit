#ifndef NAVITQUICKPROXY_H
#define NAVITQUICKPROXY_H

#include <QtCore/QObject>
#include <QtCore/QRect>
#include <QtQml/QQmlListProperty>
#include <memory>

#include "appsettings.h"
#include "imapdownloader.h"
#include "navitmapsproxy.h"
#include "locationproxy.h"

namespace NXE {
struct JSONMessage;
class NXEInstance;
}
struct Context;
class QQmlContext;

class NavitQuickProxy : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(bool enablePoi READ enablePoi WRITE setEnablePoi NOTIFY enablePoiChanged)
    Q_PROPERTY(bool ftu READ ftu WRITE setFtu NOTIFY ftuChanged)
    Q_PROPERTY(QObject* currentlySelectedItem READ currentlySelectedItem NOTIFY currentlySelectedItemChanged)

public:
    explicit NavitQuickProxy(const QString& socketName, QQmlContext* ctx, QObject *parent = 0);

    int orientation();
    void setOrientation(int);

    QString version() const;
    QString position() const;

    bool enablePoi() const;
    void setEnablePoi(bool enable);

    bool ftu() const;
    void setFtu(bool value);

    QObject* currentlySelectedItem() const;
    QObject* navitMapsProxy() {return &mapsProxy;}

    void resize(const QRect& rect);

signals:
    void orientationChanged();
    void enablePoiChanged();
    void ftuChanged();

    void quitSignal();

    void searchDone();
    void gettingHistoryDone();
    void currentlySelectedItemChanged();
    void topBarLocationVisibleChanged();

    void pointClicked(LocationProxy* location);

    // Navigation
    void navigationStarted();
    void navigationStopped();
    void navigationManuver(const QString& manuver);

public slots:
    void render();
    void reset();
    void quit();

    QString valueFor(const QString& optionName);
    void changeValueFor(const QString& optionName, const QVariant& newVal);

    // Search API
    void startSearch();
    void finishSearch();
    void searchCountry(const QString& name);
    void searchCity(const QString& name);
    void searchStreet(const QString& street);
    void searchAddress(const QString& street);
    void searchSelect(const QString& what, int id);
    void searchNear(const QString& str);

    void moveToCurrentPosition();

    void getFavorites();
    void getHistory();
    void setLocationPopUp(const QUuid& id);

    // Navigation API
    void startNavigation();
    void cancelNavigation();

    void setZoom(int newZoom);

private slots:
    void initNavit();
    void synchronizeNavit();
private:

    std::shared_ptr<Context> context;
    std::shared_ptr<NXE::NXEInstance> nxeInstance;
    QQmlContext* m_rootContext;
    AppSettings m_settings;
    NavitMapsProxy mapsProxy;
    QObjectList m_countriesSearchResults;
    QObjectList m_citiesSearchResults;
    QObjectList m_streetsSearchResults;
    QObjectList m_addressSearchResults;
    QObjectList m_favoritesResults;
    QObjectList m_historyResults;
    QScopedPointer<LocationProxy> m_currentItem;
};

#endif // NAVITQUICKPROXY_H
