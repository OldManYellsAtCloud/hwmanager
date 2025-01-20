#include "button/volumebutton.h"
#include <poll.h>
#include <linux/input.h>
#include <loglib/loglib.h>
#include "utils/eventutils.h"

VolumeButton::VolumeButton(sdbus::IObject* sdbus, sdbus::InterfaceName sdbusInterface) {
    _sdbus = sdbus;
    _sdbusInterface = sdbusInterface;

    _sdbus->addVTable(sdbus::SignalVTableItem{sdbus::MethodName{VOLUME_DOWN_PRESS}, {}, {}}).forInterface(_sdbusInterface);
    _sdbus->addVTable(sdbus::SignalVTableItem{sdbus::MethodName{VOLUME_DOWN_RELEASE}, {}, {}}).forInterface(_sdbusInterface);
    _sdbus->addVTable(sdbus::SignalVTableItem{sdbus::MethodName{VOLUME_UP_PRESS}, {}, {}}).forInterface(_sdbusInterface);
    _sdbus->addVTable(sdbus::SignalVTableItem{sdbus::MethodName{VOLUME_UP_RELEASE}, {}, {}}).forInterface(_sdbusInterface);

    std::optional<std::string> eventId = findEventID(VOLUME_BUTTON_PHYS);
    if (!eventId.has_value()){
        LOG_ERROR("Could not determine the event ID of volume button!");
        exit(1);
    }

    std::string filepath = std::format("/dev/input/{}", eventId.value());
    volumeButtonFile = fopen(filepath.c_str(), "r");

    if (!volumeButtonFile) {
        LOG_ERROR_F("Could not open {}: {}", filepath, strerror(errno));
        exit(1);
    }
}

VolumeButton::~VolumeButton()
{
    close(fileno(volumeButtonFile));
}

void VolumeButton::run(std::stop_token st){
    struct pollfd pfd[1];
    pfd[0].fd = fileno(volumeButtonFile);
    pfd[0].events = POLLIN;

    nfds_t nfds = 1;

    struct input_event event;

    while (!st.stop_requested()){
        poll(pfd, nfds, 2000);
        if (pfd[0].revents & POLLIN) {
            fread(&event, sizeof(input_event), 1, volumeButtonFile);

            LOG_DEBUG_F("Volume {} {}. Value: {}, time: {}.{}", (event.code == KEY_VOLUMEDOWN ? "down " : "up "),
                (event.type == EV_KEY ? "pressed." : "released."),
                event.value, event.time.tv_sec, event.time.tv_usec);

            if (event.type == EV_KEY && event.value == 1) {
                if (event.code == KEY_VOLUMEDOWN) {
                    sendSignal(VOLUME_DOWN_PRESS);
                } else {
                    sendSignal(VOLUME_UP_PRESS);
                }
            } else if (event.type == EV_KEY && event.value == 0) {
                if (event.code == KEY_VOLUMEDOWN) {
                    sendSignal(VOLUME_DOWN_RELEASE);
                } else {
                    sendSignal(VOLUME_UP_RELEASE);
                }
            }
        }
    }
}

void VolumeButton::sendSignal(std::string signalType)
{
    auto signal = _sdbus->createSignal(_sdbusInterface, sdbus::SignalName{signalType});
    _sdbus->emitSignal(signal);
}
