#include "button/powerbutton.h"
#include <poll.h>
#include <linux/input.h>
#include <loglibrary.h>
#include "utils/eventutils.h"

PowerButton::PowerButton(sdbus::IObject* sdbus, sdbus::InterfaceName sdbusInterface) {
    _sdbus = sdbus;
    _sdbusInterface = sdbusInterface;

    _sdbus->addVTable(sdbus::SignalVTableItem{sdbus::MethodName{POWER_BUTTON_PRESS}, {}, {}}).forInterface(_sdbusInterface);
    _sdbus->addVTable(sdbus::SignalVTableItem{sdbus::MethodName{POWER_BUTTON_RELEASE}, {}, {}}).forInterface(_sdbusInterface);

    std::optional<std::string> eventId = findEventID(POWER_BUTTON_PHYS);
    if (!eventId.has_value()){
        ERROR("Could not determine the event ID of power button!");
        exit(1);
    }

    std::string filepath = std::format("/dev/input/{}", eventId.value());
    powerButtonFile = fopen(filepath.c_str(), "r");

    if (!powerButtonFile) {
        ERROR("Could not open {}: {}", filepath, strerror(errno));
        exit(1);
    }
}

PowerButton::~PowerButton()
{
    close(fileno(powerButtonFile));
}

void PowerButton::run(std::stop_token st){
    struct pollfd pfd[1];
    pfd[0].fd = fileno(powerButtonFile);
    pfd[0].events = POLLIN;

    nfds_t nfds = 1;

    struct input_event event;

    while (!st.stop_requested()){
        poll(pfd, nfds, 2000);
        if (pfd[0].revents & POLLIN) {
            fread(&event, sizeof(input_event), 1, powerButtonFile);

            DBG("Power event - code: {}, type: {}, value: {}", event.code, event.type, event.value);

            if (event.type == EV_KEY && event.code == KEY_POWER) {
                std::string signalType = event.value == 1 ? POWER_BUTTON_PRESS : POWER_BUTTON_RELEASE;
                auto signal = _sdbus->createSignal(_sdbusInterface, sdbus::SignalName{signalType});
                _sdbus->emitSignal(signal);
            }
        }
    }
}
