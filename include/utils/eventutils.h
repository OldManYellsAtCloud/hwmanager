#ifndef EVENTUTILS_H
#define EVENTUTILS_H

#include <optional>
#include <string>

#define INPUT_DEVICES "/proc/bus/input/devices"
#define INPUT_EVENT_PATH "/dev/input/{}"
#define PHYS_TYPE "P: Phys"
#define TOUCHSCREEN_TYPE "input/ts"
#define HANDLER_TYPE "H: Handlers"
#define EVENT_HANDLER "event"

std::optional<std::string> findEventID(std::string eventType);

#endif // EVENTUTILS_H
