#include "navitquickproxy.h"
#include "nxe_instance.h"
#include "alog.h"
#include "navitdbus.h"
#include "navitprocessimpl.h"
#include "gpsdprovider.h"
#include "mapdownloaderdbus.h"
#include "dbuscontroller.h"
#include "speechimpldbus.h"
#include "nxe_version.h"
#include "mapinfoproxy.h"
#include "locationproxy.h"

#include <functional>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <QtCore/QVariant>
#include <QtQml/QQmlContext>
#include <QtCore/QTimer>
#include <QtCore/QRect>

struct Context {
    NXE::DBusController dbusController;
    NXE::INavitIPC* ipc{ new NXE::NavitDBus{ dbusController } };
    NXE::IMapDownloader* md{ new NXE::MapDownloaderDBus{ dbusController } };
#if defined(NXE_OS_LINUX)
    NXE::ISpeech* speech{ nullptr };
#elif defined(NXE_OS_TIZEN)
    NXE::ISpeech* speech{ new NXE::SpeechImplDBus{ dbusController } };
#endif
    NXE::DI::Injector injector{ std::make_tuple(
        std::shared_ptr<NXE::INavitIPC>(ipc),
        std::shared_ptr<NXE::INavitProcess>(new NXE::NavitProcessImpl),
        std::shared_ptr<NXE::IGPSProvider>(new NXE::GPSDProvider),
        std::shared_ptr<NXE::IMapDownloader>(md),
        std::shared_ptr<NXE::ISpeech>(speech)) };
};

void clearList(QObjectList& l, const QString& modelName, QQmlContext* c)
{
    QObjectList tmp = l;
    l.clear();
    c->setContextProperty(modelName, QVariant::fromValue(l));
    qDeleteAll(tmp);
}

NavitQuickProxy::NavitQuickProxy(const QString& socketName, QQmlContext* ctx, QObject* parent)
    : QObject(parent)
    , context(new Context)
    , nxeInstance(new NXE::NXEInstance{ context->injector })
    , m_rootContext(ctx)
    , mapsProxy(nxeInstance, ctx)
    , m_distance(-1)
    , m_eta(-1)
{
    nxeInstance->setWaylandSocketName(socketName.toLatin1().data());

    nxeInstance->pointClickedSignal().connect([this](const NXE::PointClicked& pc) {

        QString name, description;

        // We may have more than one entry on click (town, street and some poi)
        // The order of checking is (the least important is checked sooner)
        // 1.) street
        // 2.) poi
        // 3.) town

        // is this a street ?
        auto streetIter = std::find_if(pc.items.begin(), pc.items.end(), [](const std::pair<std::string, std::string>& p) -> bool {
            return boost::algorithm::starts_with(p.first, "street_");
        });
        if (streetIter != pc.items.end()) {
            name = QString::fromStdString(streetIter->second);
        }


        // is this a poi
        auto poiIter = std::find_if(pc.items.begin(), pc.items.end(), [](const std::pair<std::string, std::string>&p) ->bool {
            return boost::algorithm::starts_with(p.first, "poi_");
        });
        if(poiIter != pc.items.end()) {
            aDebug() << "we found a POI " << poiIter->second;
            name = QString::fromStdString(poiIter->second);
        }

        // is this a town
        auto townIter = std::find_if(pc.items.begin(), pc.items.end(), [](const std::pair<std::string, std::string>&p) ->bool {
            return boost::algorithm::starts_with(p.first, "town_label_");
        });
        if(townIter != pc.items.end()) {
            aDebug() << "we found a town " << townIter->second;
            name = QString::fromStdString(townIter->second);
        }

        // if name is still empty and we have only one entry
        if (name.isEmpty() && pc.items.size() == 1) {
            name = QString::fromStdString(pc.items.front().second);
        }

        aDebug() << "Name = " << name.toStdString() << " position = " << pc.position.longitude << " " << pc.position.latitude;
        auto loc = new LocationProxy { name, false, "1234 N Main, Portland, OR 97208", false};
        // move to parent thread
        loc->setPosition(pc.position);
        loc->moveToThread(this->thread());
        connect(loc, &LocationProxy::favoriteChanged, [this, &loc](){

            if (loc->favorite()){
                m_settings.addToFavorites(loc);
            } else {
                m_settings.removeFromFavorites(loc->id().toByteArray().data());
            }
        });
        m_currentItem.reset(loc);
        emit currentlySelectedItemChanged();
    });

    nxeInstance->ipc()->routingSignal().connect([this](const std::string& manuver) {
        emit navigationManuver(QString::fromStdString(manuver));
    });

    // Navit dbus responses

    nxeInstance->ipc()->searchResponse().connect([this](NXE::SearchResults results, NXE::INavitIPC::SearchType type) {
        if (type == NXE::INavitIPC::SearchType::Country) {
            for (NXE::SearchResult result : results) {
                aTrace() << "result " << result.country.name;
                m_countriesSearchResults.append(new LocationProxy{ result });
            }
            aDebug() << "Country Model size = " << m_countriesSearchResults.size() << static_cast<void*>(m_rootContext);
            m_rootContext->setContextProperty("countrySearchResult", QVariant::fromValue(m_countriesSearchResults));
        }
        else if (type == NXE::INavitIPC::SearchType::City) {
            for (NXE::SearchResult city : results) {
                m_citiesSearchResults.append(new LocationProxy{ city });
            }
            aDebug() << "City Model size = " << m_citiesSearchResults.size();
            m_rootContext->setContextProperty("citySearchResult", QVariant::fromValue(m_citiesSearchResults));
        }
        else if (type == NXE::INavitIPC::SearchType::Street) {

            for (NXE::SearchResult street : results) {
                m_streetsSearchResults.append(new LocationProxy{ street });
            }
            aDebug() << "Street Model size = " << m_streetsSearchResults.size();
            m_rootContext->setContextProperty("streetSearchResult", QVariant::fromValue(m_streetsSearchResults));
        }
        else if(type == NXE::INavitIPC::SearchType::Address ){
            for (NXE::SearchResult result : results) {
                m_addressSearchResults.append(new LocationProxy{ result });
            }
            aDebug() << "Address Model size = " << m_addressSearchResults.size();
            m_rootContext->setContextProperty("addressSearchResult", QVariant::fromValue(m_addressSearchResults));
        }
        emit searchDone();
    });

    nxeInstance->ipc()->distanceResponse().connect([this] (std::int32_t distance) {
        m_distance = distance;
        emit distanceToDestinationChanged();
    });

    nxeInstance->ipc()->navigationChanged().connect([this](bool navi) {
        aInfo() << "Navigation info changed to " << (navi ? "true":"false");
        emit navigationChanged();
    });

    nxeInstance->ipc()->distanceResponse().connect([this] (std::int32_t eta) {
        m_eta = eta;
        emit etaChanged();
    });

    qRegisterMetaType<QObjectList>("QObjectList");
    typedef QQmlListProperty<LocationProxy> LocationProxyList;
    qRegisterMetaType<LocationProxyList>("QQmlListProperty<LocationProxy>");

    QTimer::singleShot(500, this, SLOT(initNavit()));
}

NavitQuickProxy::~NavitQuickProxy()
{
    aDebug() << __PRETTY_FUNCTION__;
    nxeInstance->setPositionUpdateListener(0);
}

int NavitQuickProxy::orientation()
{
    return m_settings.get<Tags::Orientation>();
}

void NavitQuickProxy::setOrientation(int orientation)
{
    aDebug() << "Setting orientation to " << orientation;
    try {
        nxeInstance->ipc()->setOrientation(orientation);
        m_settings.set<Tags::Orientation>(orientation);
        aTrace() << "New orientation is " << m_settings.get<Tags::Orientation>();
        emit orientationChanged();

        QTimer::singleShot(500, [this]() {
            nxeInstance->ipc()->render();
        });
    }
    catch (const std::exception& ex) {
        aError() << "An error happened during setting orientation. Ex= " << ex.what();
    }
}

QString NavitQuickProxy::version() const
{
    return QString::fromStdString(gNXEVersion);
}

bool NavitQuickProxy::enablePoi() const
{
    return m_settings.get<Tags::EnablePoi>();
}

void NavitQuickProxy::resize(const QRect& rect)
{
    nxeInstance->resize(rect.width(), rect.height());
}

void NavitQuickProxy::setNavigation(bool start)
{
    if (start) {
        aInfo() << "Starting Navigation for " << static_cast<void*>(m_currentItem.data());
        nxeInstance->startNavigation(m_currentItem->longitude(), m_currentItem->latitude(),
                                           m_currentItem->description().toStdString());
    } else {
        nxeInstance->cancelNavigation();
    }
}

bool NavitQuickProxy::navigation()
{
    return nxeInstance->ipc()->isNavigationRunning();
}

void NavitQuickProxy::setEnablePoi(bool enable)
{
    nxeInstance->ipc()->setScheme(enable ? "Car-JLR" : "Car-JLR-nopoi");
    m_settings.set<Tags::EnablePoi>(enable);
}

bool NavitQuickProxy::ftu() const
{
    return m_settings.get<Tags::Ftu>();
}

void NavitQuickProxy::setFtu(bool value)
{
    m_settings.set<Tags::Ftu>(value);

    if (!value) {
        // ftu change to true
        initNavit();
    }

    emit ftuChanged();
}

QObject* NavitQuickProxy::currentlySelectedItem() const
{
    return m_currentItem.data();
}

void NavitQuickProxy::render()
{
    nxeInstance->ipc()->render();
}

void NavitQuickProxy::reset()
{
    aInfo() << "Resetting all data";
    clearList(m_countriesSearchResults, "countrySearchResult", m_rootContext);
    clearList(m_citiesSearchResults, "citySearchResult", m_rootContext);
    clearList(m_streetsSearchResults, "streetSearchResult", m_rootContext);
    clearList(m_addressSearchResults, "addressSearchResult", m_rootContext);
    clearList(m_favoritesResults, "locationFavoritesResult", m_rootContext);

    m_settings.remove();
    nxeInstance->mapDownloader()->removeAllMaps();

    emit ftuChanged();
}

void NavitQuickProxy::quit()
{
    aInfo() << "Quiting application";
    nxeInstance->cancelNavigation();
    nxeInstance->ipc()->quit();

    emit quitSignal();
}

QString NavitQuickProxy::valueFor(const QString& optionName)
{
    QString ret;
    if (optionName == "enablePoi") {
        bool bRet = m_settings.get<Tags::EnablePoi>();
        aDebug() << "value for poi is " << bRet;
        ret = QString("%1").arg(bRet ? "on" : "off");
    }
    else if (optionName == "voice") {
        ret = m_settings.get<Tags::Voice>() ? "on" : "off";
    }
    else if (optionName == "perspective") {
        const std::string value = m_settings.get<Tags::MapView>();
        aTrace() << "Value for pers = " << value;
        return QString::fromStdString(value);
    }

    return ret;
}

void NavitQuickProxy::changeValueFor(const QString& optionName, const QVariant& newVal)
{
    if (optionName == "enablePoi") {
        setEnablePoi(newVal.toString() == "on");
    }
    else if (optionName == "voice") {
        m_settings.set<Tags::Voice>(newVal.toString() == "on");
        nxeInstance->setAudioMute(!(m_settings.get<Tags::Voice>()));
    }
    else if (optionName == "perspective") {
        aTrace() << "Perspective new val = " << newVal.toString().toStdString();
        m_settings.set<Tags::MapView>(newVal.toString().toStdString());
        nxeInstance->ipc()->setPitch(m_settings.get<Tags::MapView>() == "2D" ? 0 : 30);
    }
}

void NavitQuickProxy::startSearch()
{
    finishSearch();
    nxeInstance->ipc()->startSearch();
}

void NavitQuickProxy::finishSearch()
{
    nxeInstance->ipc()->finishSearch();
}

void NavitQuickProxy::searchCountry(const QString& countryName)
{
    clearList(m_countriesSearchResults, "countrySearchResult", m_rootContext);
    aDebug() << "Search for country = " << countryName.toStdString();
    nxeInstance->ipc()->search(NXE::INavitIPC::SearchType::Country, countryName.toStdString());
}

void NavitQuickProxy::searchCity(const QString& name)
{
    clearList(m_citiesSearchResults, "citySearchResult", m_rootContext);
    aDebug() << "Search for city = " << name.toStdString();
    nxeInstance->ipc()->search(NXE::INavitIPC::SearchType::City, name.toStdString());
}

void NavitQuickProxy::searchStreet(const QString& street)
{
    clearList(m_streetsSearchResults, "streetSearchResult", m_rootContext);
    aDebug() << "Search for street = " << street.toStdString();
    nxeInstance->ipc()->search(NXE::INavitIPC::SearchType::Street, street.toStdString());
}

void NavitQuickProxy::searchAddress(const QString& street)
{
    clearList(m_addressSearchResults, "addressSearchResult", m_rootContext);
    aDebug() << "Search for address = " << street.toStdString();
    nxeInstance->ipc()->search(NXE::INavitIPC::SearchType::Address, street.toStdString());
}

void NavitQuickProxy::searchSelect(const QString& what, int id)
{
    if (id == -1) {
        aInfo() << "Id is -1, so no select";
        return;
    }

    aInfo() << "Selecting id= " << id;
    NXE::INavitIPC::SearchType type;
    if (what == "country") {
        type = NXE::INavitIPC::SearchType::Country;
    }
    else if (what == "city") {
        type = NXE::INavitIPC::SearchType::City;
    }
    else if (what == "street") {
        type = NXE::INavitIPC::SearchType::Street;
    }
    else if (what == "address") {
        type = NXE::INavitIPC::SearchType::Address;
    }
    else {
        throw std::runtime_error("Shouldn't happen");
    }
    nxeInstance->ipc()->selectSearchResult(type, id);
}

void NavitQuickProxy::searchNear(const QString& str)
{
    // TODO: Implement me
}

void NavitQuickProxy::moveToCurrentPosition()
{
    nxeInstance->ipc()->setTracking(true);

    auto pos = nxeInstance->gps()->position();
    nxeInstance->ipc()->setPosition(pos.longitude, pos.latitude);
}

void NavitQuickProxy::getFavorites()
{
    clearList(m_favoritesResults, "locationFavoritesResult", m_rootContext);

    auto favs = m_settings.favorites();
    aTrace() << "favorites size=" << favs.size();
    std::for_each(favs.begin(), favs.end(), [this](LocationProxy* p) {
        m_favoritesResults.append(p);
    });

    m_rootContext->setContextProperty("locationFavoritesResult", QVariant::fromValue(m_favoritesResults));
}
void NavitQuickProxy::getHistory()
{
    aFatal() << "Not implemented " << __PRETTY_FUNCTION__;

    m_historyResults.append(new LocationProxy{ "hist_test1", false, "", true });

    m_rootContext->setContextProperty("locationHistoryResult", QVariant::fromValue(m_historyResults));

    emit gettingHistoryDone();
}

void NavitQuickProxy::setZoom(int newZoom)
{
    nxeInstance->ipc()->setZoom(newZoom);
}

void NavitQuickProxy::setLocationPopUp(const QUuid& id)
{
    aDebug() << Q_FUNC_INFO;
    QObjectList tmp;
    tmp.append(m_citiesSearchResults);
    tmp.append(m_streetsSearchResults);
    tmp.append(m_addressSearchResults);
    tmp.append(m_favoritesResults);
    int newZoomLevel = -1;
    QObject* foundItem = nullptr;
    std::for_each(tmp.begin(), tmp.end(), [this, &id, &foundItem](QObject* o) {
        LocationProxy* proxy = qobject_cast<LocationProxy*>(o);

        if (proxy->id() == id) {
            // we have to copy this, since in a second the model will be deleted
            // and this will points to an deleted object
            foundItem = proxy;
            m_currentItem.reset(LocationProxy::clone(proxy));
        }
    });

    if (!m_currentItem) {
        aFatal() << "Unable to find item= " << id.toByteArray().data();
        return;
    }
    emit currentlySelectedItemChanged();
    connect(m_currentItem.data(), &LocationProxy::favoriteChanged, [this]() {
            aInfo() << "Adding " << m_currentItem->id().toByteArray().data() << " to favs";
            if (m_currentItem->favorite())
                m_settings.addToFavorites(m_currentItem.data());
            else
                m_settings.removeFromFavorites(m_currentItem->id().toByteArray().data());
    });

    // disable tracking

    nxeInstance->ipc()->setTracking(false);

    aInfo() << "Setting location to " << m_currentItem->longitude() << " " << m_currentItem->latitude();
    nxeInstance->ipc()->setPosition(m_currentItem->longitude(), m_currentItem->latitude());
    if (m_citiesSearchResults.contains(foundItem)) {
        newZoomLevel = 4096;
    }
    else if (m_streetsSearchResults.contains(foundItem))
        newZoomLevel = 16;

    else if (m_addressSearchResults.contains(foundItem))
        newZoomLevel = 8;

    aDebug() << " new zoom level = " << newZoomLevel;
    if (newZoomLevel != -1) {
        QTimer::singleShot(100, [this, newZoomLevel]() {
                nxeInstance->ipc()->setZoom(newZoomLevel);
        });
    }
}

void NavitQuickProxy::initNavit()
{
    context->dbusController.start();

    m_rootContext->setContextProperty("countrySearchResult", QVariant::fromValue(m_countriesSearchResults));
    m_rootContext->setContextProperty("citySearchResult", QVariant::fromValue(m_citiesSearchResults));
    m_rootContext->setContextProperty("streetSearchResult", QVariant::fromValue(m_streetsSearchResults));
    m_rootContext->setContextProperty("addressSearchResult", QVariant::fromValue(m_addressSearchResults));
    m_rootContext->setContextProperty("locationFavoritesResult", QVariant::fromValue(m_favoritesResults));
    if (m_settings.get<Tags::Ftu>()) {
        aInfo() << "Ftu is set up no navit need to be run";
        return;
    }
    aInfo() << "Launching navit";

    nxeInstance->Initialize();
    QTimer::singleShot(500, this, SLOT(synchronizeNavit()));
}

void NavitQuickProxy::synchronizeNavit()
{
    // TODO: Synchronize all NavIt settings
    aInfo() << "Synchronizing navit";
    if (m_settings.get<Tags::Ftu>()) {
        return;
    }
    // special case
    nxeInstance->resize(0, 0);

    // set scheme
    setEnablePoi(m_settings.get<Tags::EnablePoi>());
    setOrientation(m_settings.get<Tags::Orientation>());

    // audio
    nxeInstance->setAudioMute(!(m_settings.get<Tags::Voice>()));
    nxeInstance->ipc()->setPitch(m_settings.get<Tags::MapView>() == "2D" ? 0 : 30);
}
