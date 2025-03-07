#include <string.h>
#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <linux/input-event-codes.h>
#include <loglib/loglib.h>
#include <format>

#include "touchscreen/touchscreen.h"
#include "utils/eventutils.h"

using namespace TouchScreenNS;

TouchScreen::TouchScreen(sdbus::IObject* sdbus, sdbus::InterfaceName sdbusInterface)
{
    _sdbus = sdbus;
    _sdbusInterface = sdbusInterface;
    registerDbusSignals();

    std::optional<std::string> touchScreenEvent = findEventID(TOUCHSCREEN_TYPE);

    if (!touchScreenEvent.has_value()){
        LOG_ERROR("Could not extract touch screen input name!");
        exit(1);
    }

    std::string touchScreenPath = std::format(INPUT_EVENT_PATH, touchScreenEvent.value());

    eventSource = fopen(touchScreenPath.c_str(), "r");

    if (!eventSource) {
        LOG_ERROR_F("Could not open event source: {}, error: {}.", touchScreenPath, strerror(errno));
        exit(1);
    }

    pfd[0].fd = fileno(eventSource);
    pfd[0].events = POLLIN;
}

TouchScreen::~TouchScreen()
{
    close(fileno(eventSource));
}

void TouchScreen::registerDbusSignals(){
    for (const std::string& s: directionStrings){
        _sdbus->addVTable(sdbus::SignalVTableItem{sdbus::MethodName{s}, {}, {}}).forInterface(_sdbusInterface);
    }
}

void TouchScreen::sendSignal(Direction direction){
    std::string signalType = directionStrings[direction];

    auto signal = _sdbus->createSignal(_sdbusInterface, sdbus::SignalName{signalType});
    _sdbus->emitSignal(signal);
}

bool TouchScreen::readEvent(struct input_event& event)
{
    poll(pfd, 1, POLL_TIMEOUT_MS);
    if (pfd[0].revents & POLLIN){
        fread(&event, sizeof(event), 1, eventSource);
        return true;
    }
    return false;
}

Direction TouchScreen::calculateDirection(struct Points& points){

    Direction ret = Direction::INVALID;

    int horizontalDiff = std::abs(points.start.x - points.end.x);

    LOG_DEBUG_F("Calculating direction x1: {}, x2: {}, y1: {}, y2: {}. HorizontalDiff: {}", points.start.x, points.end.x,
        points.start.y, points.end.y, horizontalDiff);

    if (horizontalDiff < 50) { // horizontal movement is very small, it is a vertical movement
        int verticalMovement = points.end.y  - points.start.y;
        LOG_DEBUG_F("Vertical movement detected, points.end.y  - points.start.y: {}", verticalMovement);

        if ( verticalMovement < 0 ) {
            if (points.start.y > (BOTTOM_Y - SCREEN_MARGIN)) {
                LOG_DEBUG_F("BOTTOM_TO_TOP movement");
                ret =  Direction::BOTTOM_TO_TOP;
            }
        } else if (points.start.y < (TOP_Y + SCREEN_MARGIN)) {
            LOG_DEBUG_F("TOP_TO_BOTTOM movement");
            ret =  Direction::TOP_TO_BOTTOM;
        }
    } else { //horizontal movement
        int horizontalMovement = points.end.x - points.start.x;
        LOG_DEBUG_F("Horizontal movement detected, points.end.x - points.start.x: {}", horizontalMovement);

        if (horizontalMovement > 0) {
            if (points.start.x < SCREEN_MARGIN) {
                LOG_DEBUG_F("LEFT_TO_RIGHT movement");
                ret =  Direction::LEFT_TO_RIGHT;
            }
        } else if (points.start.x > (RIGHT_X - SCREEN_MARGIN)) {
            LOG_DEBUG_F("RIGHT_TO_LEFT movement");
            ret =  Direction::RIGHT_TO_LEFT;
        }
    }

    return ret;
}

std::optional<struct Points> TouchScreen::collectData(std::stop_token& stopToken){
    std::optional<struct Points> ret;
    size_t eventNumX = 0;
    size_t eventNumY = 0;

    int readTries = 0;
    const int MAX_READ_TRIES = 3;

    Points points;

    bool enoughDataCollected = false;
    bool gestureStarted = false;

    struct input_event event;

    do {
        if (readEvent(event)){
            readTries = 0;
        } else {
            ++readTries;
        }

        if (readTries >= MAX_READ_TRIES)
            break;

        bool touchEndpoint = event.code == BTN_TOUCH && event.type == EV_KEY;

        if (!gestureStarted){
            gestureStarted = event.value == 1 && touchEndpoint;
            eventNumX = 0;
            eventNumY = 0;
        } else {
            if (event.code == ABS_X && event.type == EV_ABS){
                if (eventNumX++ == 0) points.start.x = event.value;
                points.end.x = event.value;
            } else if (event.code == ABS_Y && event.type == EV_ABS){
                if (eventNumY++ == 0) points.start.y = event.value;
                points.end.y = event.value;

            }
        }

        enoughDataCollected = eventNumX > 0 &&
                              eventNumY > 0 &&
                              (eventNumX + eventNumY) > MIN_EVENT_NUMBER;

    } while (!stopToken.stop_requested() && !enoughDataCollected);

    if (enoughDataCollected)
        ret = points;

    return ret;
}

void TouchScreen::run(std::stop_token stopToken)
{
    while (!stopToken.stop_requested()){
        // collecting points might take a while, forward the token to end it quick, if needed
        std::optional<struct Points> points = collectData(stopToken);
        if (points.has_value()){
            Direction dir = calculateDirection(points.value());
            if (dir != Direction::INVALID) sendSignal(dir);
        }
    }
}

