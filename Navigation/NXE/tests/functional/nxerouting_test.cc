#include "nxe_instance.h"
#include "navitprocessimpl.h"
#include "navitcontroller.h"
#include "navitdbus.h"
#include "mapdownloaderdbus.h"
#include "gpsdprovider.h"
#include "testutils.h"
#include "dbuscontroller.h"
#include "speechimpl.h"

#include <gtest/gtest.h>
#include <memory>
#include <chrono>
#include <thread>
#include <fruit/fruit.h>

using namespace NXE;
extern bool runNavit;

typedef fruit::Component<INavitIPC, INavitProcess, IGPSProvider> NXEImpls;
struct NXEInstanceTest : public ::testing::Test {

    DBusController dbusController;
    INavitIPC * ipc { new NavitDBus{dbusController}};
    IMapDownloader * md { new MapDownloaderDBus{dbusController}};
    DI::Injector injector{ [this]() -> DI::Components {
        return fruit::createComponent()
                .bindInstance(*ipc)
                .bindInstance(*md)
                .bind<INavitProcess, NavitProcessImpl>()
                .bind<IGPSProvider, GPSDProvider>()
                .bind<ISpeech,SpeechImpl>();
    }() };
    NXEInstance instance{ injector };
    JSONMessage respMsg;
    bool receivedRender{ false };
    std::size_t numberOfResponses = 0;

    static void SetUpTestCase()
    {
        TestUtils::createNXEConfFile();
    }

    void callback(const JSONMessage& resp)
    {
        respMsg = resp;
        numberOfResponses++;
    }

    void setDestination(double lon, double lat, const char* desc)
    {
         instance.HandleMessage(TestUtils::setDestinationMessage(lon,lat,desc));
    }

    void setPosition(double lon, double lat)
    {
         instance.HandleMessage(TestUtils::setPositionMessage(lon,lat));
    }

    void clearDestination()
    {
        instance.HandleMessage(TestUtils::clearDestinationMessage());
    }
};


TEST_F(NXEInstanceTest, Routing)
{
    instance.registerMessageCallback(std::bind(&NXEInstanceTest::callback, this, std::placeholders::_1));
    instance.Initialize();

    setPosition(11.5659, 48.1392);

    ASSERT_EQ(respMsg.call, "setPosition");
    EXPECT_TRUE(respMsg.data.empty());

    std::chrono::milliseconds dura_1s(1000);
    std::this_thread::sleep_for(dura_1s);

    setDestination(11.5775, 48.1427, "1");

    ASSERT_EQ(respMsg.call, "setDestination");
    EXPECT_TRUE(respMsg.data.empty());

    std::chrono::milliseconds dura_5s(5000);
    std::this_thread::sleep_for(dura_5s);

    clearDestination();
    ASSERT_EQ(respMsg.call, "clearDestination");
    EXPECT_TRUE(respMsg.data.empty());

    setDestination(11.5875, 48.1527, "2");

    ASSERT_EQ(respMsg.call, "setDestination");
    EXPECT_TRUE(respMsg.data.empty());

    std::this_thread::sleep_for(dura_5s);

    setPosition(11.5859, 48.1692);

    ASSERT_EQ(respMsg.call, "setPosition");
    EXPECT_TRUE(respMsg.data.empty());

    std::this_thread::sleep_for(dura_1s);

    setDestination(11.5975, 48.1727, "3");

    ASSERT_EQ(respMsg.call, "setDestination");
    EXPECT_TRUE(respMsg.data.empty());

    std::this_thread::sleep_for(dura_5s);

    clearDestination();
    ASSERT_EQ(respMsg.call, "clearDestination");
    EXPECT_TRUE(respMsg.data.empty());
}
