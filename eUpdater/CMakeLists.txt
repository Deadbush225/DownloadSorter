cmake_minimum_required(VERSION 3.16)
project(eUpdater VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Core Widgets Network REQUIRED)

# Main updater executable
add_executable(eUpdater
    src/main.cpp
    src/updater.cpp
    src/updater.h
)

target_link_libraries(eUpdater PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
)

# Installation
include(GNUInstallDirs)
install(TARGETS eUpdater
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Package configuration for consuming projects
include(CMakePackageConfigHelpers)

# Generate package config files
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/eUpdaterConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/eUpdaterConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/eUpdater
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/eUpdaterConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Install package config files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/eUpdaterConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/eUpdaterConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/eUpdater
)

# For packaging/distribution
set(CPACK_PACKAGE_NAME "eUpdater")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "eUpdater - Qt-based updater utility")
include(CPack)
