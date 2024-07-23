#ifndef TOUCHSCREEN_H
#define TOUCHSCREEN_H

#include <string>
#include <poll.h>
#include <linux/input.h>
#include <sdbus-c++/sdbus-c++.h>

#define SCREEN_MARGIN 30
#define TOP_Y 0
#define BOTTOM_Y 1439
#define LEFT_X 0
#define RIGHT_X 719

#define POLL_TIMEOUT_MS 2000

#define MIN_EVENT_NUMBER 20

namespace TouchScreenNS {

enum Direction {
    LEFT_TO_RIGHT = 0,
    RIGHT_TO_LEFT,
    TOP_TO_BOTTOM,
    BOTTOM_TO_TOP,
    INVALID
};

struct Point {
    int x, y;
};

struct Points {
    struct Point start, end;
};

class TouchScreen
{
private:
    FILE* eventSource;
    struct pollfd pfd[1];

    sdbus::IObject* _sdbus;
    sdbus::InterfaceName _sdbusInterface;
    void registerDbusSignals();

    Direction calculateDirection(struct Points& points);
    std::optional<struct Points> collectData(std::stop_token& stopToken);
    void sendSignal(Direction direction);

    bool readEvent(struct input_event& event);

    const std::vector<std::string> directionStrings = {
        "LEFT_TO_RIGHT", "RIGHT_TO_LEFT", "TOP_TO_BOTTOM", "BOTTOM_TO_TOP"
    };

public:

    TouchScreen(sdbus::IObject* sdbus, sdbus::InterfaceName sdbusInterface);
    ~TouchScreen(){}

    void run(std::stop_token stopToken);
};
} // end of TouchScreen namespace
#endif // TOUCHSCREEN_H
