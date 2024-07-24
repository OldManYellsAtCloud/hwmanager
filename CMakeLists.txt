cmake_minimum_required(VERSION 3.5)

project(hwmanager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(enable_tests "Build tests" OFF)

find_package(sdbus-c++ REQUIRED)
find_package(SettingsLib)
find_package(LogLibrary)

set(SOURCES src/main.cpp
            src/touchscreen/touchscreen.cpp
            src/utils/eventutils.cpp
            src/button/powerbutton.cpp
            src/button/volumebutton.cpp)

set(HEADERS include/touchscreen/touchscreen.h
            include/utils/eventutils.h
            include/button/powerbutton.h
            include/button/volumebutton.h)

add_executable(hwmanager ${HEADERS} ${SOURCES})

include(GNUInstallDirs)
install(TARGETS hwmanager
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

target_include_directories(hwmanager PRIVATE include)
target_link_libraries(hwmanager PRIVATE SDBusCpp::sdbus-c++)

if (enable_tests)
    find_package(GTest REQUIRED)
    enable_testing()
    include(GoogleTest)
endif()
