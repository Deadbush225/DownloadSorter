cmake_minimum_required(VERSION 3.14)
project(Updater LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets Network REQUIRED)

add_executable(Updater
    updater.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/icons/icons.qrc"
)

set_target_properties(Updater PROPERTIES OUTPUT_NAME "Updater")
target_link_libraries(Updater PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

# Install the updater next to the main app (respects parent CMAKE_INSTALL_PREFIX when added via add_subdirectory)
install(TARGETS Updater DESTINATION .)

# Only create a standalone install_local target when building Updater as the top-level project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(INSTALL_DIR "${CMAKE_SOURCE_DIR}/../install")
    add_custom_target(install_local
        COMMAND ${CMAKE_COMMAND} --install "${CMAKE_BINARY_DIR}" --prefix "${INSTALL_DIR}" $<$<BOOL:${CMAKE_CONFIGURATION_TYPES}>:--config> $<$<BOOL:${CMAKE_CONFIGURATION_TYPES}>:$<CONFIG>> $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:--strip>
        DEPENDS Updater
        COMMENT "Installing Updater to ${INSTALL_DIR}"
    )
endif()