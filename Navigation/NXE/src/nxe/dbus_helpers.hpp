#ifndef DBUS_HELPERS_HPP
#define DBUS_HELPERS_HPP

#include <dbus-c++/interface.h>
#include <dbus-c++/message.h>

namespace NXE {
namespace DBusHelpers {

namespace __details {
    template <typename T>
    void processOne(::DBus::MessageIter& it, T t)
    {
        it << t;
    }

    inline void unpack(::DBus::MessageIter&)
    {
    }
}

    template <typename R>
    R getAttr(const std::string& attrName, ::DBus::InterfaceProxy& proxy)
    {
        DBus::CallMessage call;
        ::DBus::MessageIter it = call.writer();
        call.member("get_attr");
        it << attrName;
        ::DBus::Message ret = proxy.invoke_method(call);

        if (ret.is_error()) {
            throw std::runtime_error("Unable to get attr ");
        }

        std::string attr;
        ::DBus::MessageIter retIter = ret.reader();
        retIter >> attr;
        ::DBus::Variant v;
        retIter >> v;
        R value = static_cast<R>(v);
        return value;
    }


    template <typename T, typename... Args>
    void unpack(::DBus::MessageIter& it, T t, Args... args)
    {
        using namespace __details;
        processOne(it, t);
        unpack(it, args...);
    }

    template <typename... Args>
    bool callNoReply(const std::string& methodName, ::DBus::InterfaceProxy& proxy, Args... attr)
    {
        using namespace __details;
        DBus::CallMessage call;
        ::DBus::MessageIter it = call.writer();
        call.member(methodName.c_str());
        unpack(it, attr...);
        return proxy.invoke_method_noreply(call);
    }

    template <typename... Args>
    ::DBus::Message call(const std::string& methodName, ::DBus::InterfaceProxy& proxy, Args... attr)
    {
        using namespace __details;
        DBus::CallMessage call;
        ::DBus::MessageIter it = call.writer();
        call.member(methodName.c_str());
        unpack(it, attr...);
        ::DBus::Message ret = proxy.invoke_method(call);

        if (ret.is_error()) {
            throw std::runtime_error("Unable to call" + methodName);
        }
        return ret;
    }

    template <typename Arg>
    void setAttr(const std::string& attrName, ::DBus::InterfaceProxy& proxy, Arg && value)
    {
        ::DBus::Variant val;
        ::DBus::MessageIter ww = val.writer();
        ww << value;
        callNoReply("set_attr", proxy, attrName, val);
    }

    template<typename T>
    T getFromIter(::DBus::MessageIter iter)
    {
        T t;
        iter >> t;
        return t;
    }

} // DBusHelpers
} // NXE

#endif // DBUS_HELPERS_HPP
