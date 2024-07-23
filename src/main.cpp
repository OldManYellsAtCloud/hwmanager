#include <vector>
#include <thread>

#include <sdbus-c++/sdbus-c++.h>
#include <touchscreen/touchscreen.h>

using namespace std;

std::vector<std::jthread> threads;


void stopThreads(){
    for (auto& t: threads)
        t.request_stop();

    for (auto& t: threads)
        t.join();
}

int main()
{
    atexit(stopThreads);

    sdbus::ServiceName serviceName{"org.gspine.hardware"};
    auto connection = sdbus::createBusConnection(serviceName);

    sdbus::ObjectPath objectPath{"/org/gspine/hardware"};
    auto sdbusObject = sdbus::createObject(*connection, std::move(objectPath));

    sdbus::InterfaceName interfaceName{"org.gspine.Hardware"};

    TouchScreenNS::TouchScreen ts{sdbusObject.get(), interfaceName};
    threads.push_back(std::jthread{&TouchScreenNS::TouchScreen::TouchScreen::run, ts});

    connection->enterEventLoop();

    return 0;
}
