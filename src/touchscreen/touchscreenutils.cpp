#include "touchscreen/touchscreenutils.h"
#include <fstream>
#include <loglibrary.h>
#include <sstream>

namespace {

bool isLineHandlerType(const std::string& line){
    return line.starts_with(HANDLER_TYPE);
}

bool isEventId(const std::string& line){
    return line.starts_with(EVENT_HANDLER);
}

std::optional<std::string> extractEventId(const std::string& line){
    std::stringstream tokens{line};
    std::optional<std::string> eventId;
    std::string tmp;

    do {
        tokens >> tmp;
    } while (!isEventId(tmp) && tokens.good());

    if (isEventId(tmp))
        eventId = tmp;

    return eventId;
}

std::optional<std::string> getNextEventId(std::ifstream& stream){
    std::string line;
    std::optional<std::string> eventId;
    while (stream.good()){
        std::getline(stream, line);
        if (isLineHandlerType(line)){
            eventId = extractEventId(line);
            break;
        }
    }
    return eventId;
}

bool isLineTouchscreenName(const std::string& line){
    return line.starts_with(PHYS_TYPE) && line.ends_with(TOUCHSCREEN_TYPE);
}

} // end of anonymous namespace

std::optional<std::string> findTouchScreenEvent(){
    std::ifstream inputDevices{INPUT_DEVICES, std::ios_base::in};
    std::optional<std::string> eventId;

    if (!inputDevices.is_open()){
        ERROR("Could not open input devices!");
        exit(1);
    }

    std::string line;

    while (inputDevices.good()){
        std::getline(inputDevices, line);
        if (isLineTouchscreenName(line)){
            eventId = getNextEventId(inputDevices);
            break;
        }
    }

    inputDevices.close();
    return eventId;
}
