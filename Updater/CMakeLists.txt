cmake_minimum_required(VERSION 3.14)
project(Updater LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets Network REQUIRED)

add_executable(Updater
    updater.cpp
)

set_target_properties(Updater PROPERTIES OUTPUT_NAME "Updater")

target_link_libraries(Updater PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

# Add installation target
install(TARGETS Updater DESTINATION bin)