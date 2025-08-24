cmake_minimum_required(VERSION 3.16)
project(DownloadSorter VERSION 0.1.0)

set(CMAKE_PREFIX_PATH "C:/Dev/Qt/6.7.3/mingw_64")
set(CMAKE_C_COMPILER "C:/Dev/Qt/Tools/mingw1120_64/bin/gcc.exe")
set(CMAKE_CXX_COMPILER "C:/Dev/Qt/Tools/mingw1120_64/bin/g++.exe")

# Find includes in the build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Turn on automatic invocation of the MOC, UIC & RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(Boost_USE_STATIC_LIBS OFF)

set(PROJECT_NAME DownloadSorter)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Core)
qt_standard_project_setup()

file(GLOB_RECURSE SourceFiles "./DownloadSorter/*.cpp")
file(GLOB_RECURSE HeaderFiles "./Include/DownloadSorter/*.h")
file(GLOB_RECURSE InstallerConfigs "../config/*.*")
file(GLOB_RECURSE Packages "../packages/*.*")
file(GLOB_RECURSE Scripts "../*.ps1")

set(SOURCE_FILES
    main.cpp
    ${SourceFiles}

    ${HeaderFiles}
    ${InstallerConfigs}
    ${Packages}
    ${Scripts}
)

# add_executable(PrintingRates main.cpp DashboardWidget.cpp)
# ---
set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/icons/Download Sorter.rc")
add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${app_icon_resource_windows} "./icons/icons.qrc")

# include_directories("src/Utils" "Utils/")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "Download Sorter")

if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    set_property(TARGET ${PROJECT_NAME} PROPERTY WIN32_EXECUTABLE true)
endif()

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# libraries
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core)

# ━━━━━━━━━━━━━━━━━━━━━━━━━ Installation ━━━━━━━━━━━━━━━━━━━━━━━━━
# Local install directory inside the project
set(INSTALL_DIR "${CMAKE_SOURCE_DIR}/../install")

# Keep default install() using this prefix (overridden by --prefix below if needed)
set(CMAKE_INSTALL_PREFIX "${INSTALL_DIR}" CACHE PATH "Install prefix" FORCE)

# Install Scripts
if(APPLE)
    set(executable_path "$<TARGET_FILE_NAME:${PROJECT_NAME}>.app")
elseif(WIN32)
    set(executable_path "\${QT_DEPLOY_BIN_DIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}>")
else()
    set(executable_path "\${QT_DEPLOY_BIN_DIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}>")
endif()

# Harder compression for Qt resources
set(CMAKE_AUTORCC_OPTIONS --compress 9 --threshold 1)

# MinGW size optimization for Release/MinSizeRel
if(MINGW)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release,MinSizeRel>:-Os -ffunction-sections -fdata-sections>
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release,MinSizeRel>:-Wl,--gc-sections -s>
    )
endif()

# Create the runtime deploy script, that gathers all the dependencies. Equivalent of "windeployqt", but with filters
set(deploy_script "${CMAKE_CURRENT_BINARY_DIR}/deploy_${PROJECT_NAME}.cmake")
file(GENERATE OUTPUT "${deploy_script}" CONTENT "
# This file is generated at generate-time and executed at install time
set(QT_DEPLOY_EXCLUDE_MODULES \"Network;NetworkInformation;NetworkAuth\")
set(QT_DEPLOY_EXCLUDE_PLUGINS \"tls;*;networkinformation;*;bearer;*;imageformats;qgif;qjpeg;qsvg;qsvgicon;generic;qtuiotouchplugin;\")
set(QT_DEPLOY_EXCLUDE_LIBRARIES \"Qt6Network;D3DCompiler_47\")
set(QT_DEPLOY_PLUGINS \"platforms;qwindows\")
include(\"${QT_DEPLOY_SUPPORT}\")
qt_deploy_runtime_dependencies(
    EXECUTABLE \"${executable_path}\"
    PLUGINS_DIR bin
    NO_TRANSLATIONS
)
")

# Install the app (goes to ${INSTALL_DIR}/bin) and deploy Qt runtime deps
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(SCRIPT ${deploy_script})

# Prune optional/banned bits that may remain or from previous installs using a script file
install(SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/PruneInstall.cmake")

# Convenience target: copy to install and run deploy (strip in Release/MinSizeRel)
add_custom_target(install_local
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${INSTALL_DIR}/bin"
    COMMAND ${CMAKE_COMMAND} --install "${CMAKE_BINARY_DIR}" --prefix "${INSTALL_DIR}" $<$<BOOL:${CMAKE_CONFIGURATION_TYPES}>:--config> $<$<BOOL:${CMAKE_CONFIGURATION_TYPES}>:$<CONFIG>> $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:--strip>
    DEPENDS ${PROJECT_NAME}
    COMMENT "Installing to ${INSTALL_DIR} and deploying Qt runtime dependencies"
)