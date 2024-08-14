#ifndef VOLUMEBUTTON_H
#define VOLUMEBUTTON_H

#include <sdbus-c++/sdbus-c++.h>

#define VOLUME_DOWN_PRESS       "volumeDownPress"
#define VOLUME_DOWN_RELEASE     "volumeDownRelease"
#define VOLUME_UP_PRESS         "volumeUpPress"
#define VOLUME_UP_RELEASE       "volumeUpRelease"

#ifdef PINEPHONE_PRO
#define VOLUME_BUTTON_PHYS  "adc-keys/input0"
#elif defined PINEPHONE
#define VOLUME_BUTTON_PHYS  "sun4i_lradc/input0"
#else
#error "Either PINEPHONE or PINEPHONE_PRO macro must be defined! Check Cmake config!"
#endif

class VolumeButton
{
    sdbus::IObject* _sdbus;
    sdbus::InterfaceName _sdbusInterface;
    FILE* volumeButtonFile;
    void sendSignal(std::string signalType);

public:
    VolumeButton(sdbus::IObject* sdbus, sdbus::InterfaceName sdbusInterface);
    ~VolumeButton();
    void run(std::stop_token st);
};

#endif // VOLUMEBUTTON_H
