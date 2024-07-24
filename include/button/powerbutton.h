#ifndef POWERBUTTON_H
#define POWERBUTTON_H

#include <sdbus-c++/sdbus-c++.h>

#define POWER_BUTTON_PRESS      "powerButtonPress"
#define POWER_BUTTON_RELEASE    "powerButtonRelease"

#define POWER_BUTTON_PHYS  "gpio-keys/input0"

class PowerButton
{
    sdbus::IObject* _sdbus;
    sdbus::InterfaceName _sdbusInterface;
    FILE* powerButtonFile;

public:
    PowerButton(sdbus::IObject* sdbus, sdbus::InterfaceName sdbusInterface);
    ~PowerButton();
    void run(std::stop_token st);
};

#endif // POWERBUTTON_H
