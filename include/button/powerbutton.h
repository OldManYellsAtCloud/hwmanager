#ifndef POWERBUTTON_H
#define POWERBUTTON_H

#include <sdbus-c++/sdbus-c++.h>

#define POWER_BUTTON_PRESS      "powerButtonPress"
#define POWER_BUTTON_RELEASE    "powerButtonRelease"

#ifdef PINEPHONE_PRO
#define POWER_BUTTON_PHYS  "gpio-keys/input0"
#elif defined PINEPHONE
#define POWER_BUTTON_PHYS  "m1kbd/input2"
#else
static_assert(false, "Either PINEPHONE or PINEPHONE_PRO macro must be defined! Check Cmake config!");
#endif

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
