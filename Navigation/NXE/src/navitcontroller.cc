#include "navitcontroller.h"
#include "navitipc.h"
#include "log.h"
#include "calls.h"

#include <functional>
#include <map>
#include <typeindex>
#include <thread>
#include <chrono>

#include <boost/lexical_cast.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/signals2/signal.hpp>

namespace bpt = boost::property_tree;

namespace NXE {

struct NavitControllerPrivate {
    std::shared_ptr<NavitIPCInterface> ipc;
    NavitController* q;
    std::thread m_retriggerThread;
    bool m_isRunning = false;
    boost::signals2::signal<void(const JSONMessage&)> successSignal;
    map_type m{ boost::fusion::make_pair<MoveByMessage>("moveBy"),
                boost::fusion::make_pair<ZoomByMessage>("zoomBy"),
                boost::fusion::make_pair<ZoomMessage>("zoom"),
                boost::fusion::make_pair<PositionMessage>("position") };

    map_cb_type cb{
        boost::fusion::make_pair<MoveByMessage>([this](const JSONMessage& message) {
            if (message.data.empty()) {
                // TODO: Change exception
                throw std::runtime_error("Unable to parse");
            }

            const int x = message.data.get<int>("x");
            const int y = message.data.get<int>("y");
            nDebug() << "IPC: Move by " << x << y;
            ipc->moveBy(x,y);

            // TODO: proper success signal
            JSONMessage response {message.id, message.call};
            successSignal(response);
        }),

        boost::fusion::make_pair<ZoomByMessage>([this](const JSONMessage& message) {
            nTrace() << "Calling zoomBy";
            int factor = message.data.get<int>("factor");
            ipc->zoomBy(factor);
        }),

        boost::fusion::make_pair<ZoomMessage>([this](const JSONMessage& message) {
            int zoomValue = ipc->zoom();
            bpt::ptree values;
            values.put("zoom", zoomValue);
            JSONMessage response {message.id, message.call, "", values };
            // TODO: proper success signal
            successSignal(response);
        }),

        boost::fusion::make_pair<PositionMessage>([this](const JSONMessage& message) {
            q->positon();
            // TODO: proper success signal
        }),
    };


    template <typename T>
    void handleMessage(const JSONMessage& data)
    {
        auto fn = boost::fusion::at_key<T>(cb);
        fn(data);
    }
};

template <class Pred, class Fun>
struct filter {
    Pred pred_;
    const Fun& fun_;

    filter(Pred p, const Fun& f)
        : pred_(p)
        , fun_(f)
    {
    }

    template <class Pair>
    void operator()(Pair& pair) const
    {
        if (pred_(pair.second))
            fun_(pair);
    }
};

template <class Pred, class Fun>
filter<Pred, Fun> make_filter(Pred p, const Fun& f)
{
    return filter<Pred, Fun>(p, f);
}

struct fun {
    fun(NXE::NavitControllerPrivate* d, const JSONMessage& data)
        : _d(d)
        , _data(data)
    {
    }

    template <class First, class Second>
    void operator()(boost::fusion::pair<First, Second>&) const
    {
        _d->handleMessage<First>(_data);
    }

    NXE::NavitControllerPrivate* _d;
    const JSONMessage& _data;
};

NavitController::NavitController(std::shared_ptr<NavitIPCInterface> ipc)
    : d(new NavitControllerPrivate)
{
    d->ipc = ipc;
    d->q = this;
}

NavitController::~NavitController()
{
}

void NavitController::positon()
{
    // TODO: Ask for LBS position
}

void NavitController::tryStart()
{
    nDebug() << "Trying to start IPC Navit controller";
    d->ipc->start();
}

void NavitController::handleMessage(const JSONMessage &msg)
{
    bool bCalled = false;
    nDebug() << "Handling message " << msg.call;
    boost::fusion::for_each(d->m, make_filter([msg, &bCalled](const std::string& str) -> bool {
        if (str == msg.call)  {
            bCalled = true;
            return true;
        }
        return false;
                                              },
                                              fun(d.get(), msg)));

    if (!bCalled) {
        nFatal() << "Unable to call " << msg.call;
        throw std::runtime_error("No parser found");
    }
}

void NavitController::addListener(const NavitController::Callback_type& cb)
{
    d->successSignal.connect(cb);
}

} // namespace NXE
