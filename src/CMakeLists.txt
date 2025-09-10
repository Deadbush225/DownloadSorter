cmake_minimum_required(VERSION 3.19)

# Read version string from repository manifest.json
file(READ "${CMAKE_SOURCE_DIR}/../manifest.json" _MANIFEST_STR)
string(JSON PROJECT_VER GET "${_MANIFEST_STR}" version)
project(DownloadSorter VERSION ${PROJECT_VER})

# Removed hardcoded Windows compiler and Qt paths to support CI environments
# if(WIN32)
# set(CMAKE_PREFIX_PATH "C:/Dev/Qt/6.7.3/mingw_64")
# set(CMAKE_C_COMPILER "C:/Dev/Qt/Tools/mingw1120_64/bin/gcc.exe")
# set(CMAKE_CXX_COMPILER "C:/Dev/Qt/Tools/mingw1120_64/bin/g++.exe")
# endif()

# Find includes in the build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Turn on automatic invocation of the MOC, UIC & RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(Boost_USE_STATIC_LIBS OFF)

set(PROJECT_NAME DownloadSorter)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core)
qt_standard_project_setup()

# Debug Qt installation
message(STATUS "=== Qt Installation Debug ===")
message(STATUS "Qt6_FOUND: ${Qt6_FOUND}")
message(STATUS "Qt6_VERSION: ${Qt6_VERSION}")
message(STATUS "Qt6_DIR: ${Qt6_DIR}")
message(STATUS "Qt6Core_DIR: ${Qt6Core_DIR}")
message(STATUS "Qt6Widgets_DIR: ${Qt6Widgets_DIR}")
get_target_property(QT_LOCATION Qt6::Core LOCATION)
message(STATUS "Qt6::Core location: ${QT_LOCATION}")
message(STATUS "================================")

# Find windeployqt for Windows deployment
if(WIN32)
    # Find windeployqt executable
    get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)

    if(QT_QMAKE_EXECUTABLE)
        get_filename_component(QT_WINDEPLOYQT_EXECUTABLE ${QT_QMAKE_EXECUTABLE} PATH)
        set(QT_WINDEPLOYQT_EXECUTABLE "${QT_WINDEPLOYQT_EXECUTABLE}/windeployqt.exe")

        if(EXISTS ${QT_WINDEPLOYQT_EXECUTABLE})
            message(STATUS "Found windeployqt: ${QT_WINDEPLOYQT_EXECUTABLE}")
            set(QT_DEPLOY_TOOL_FOUND TRUE)
        else()
            message(WARNING "windeployqt not found at: ${QT_WINDEPLOYQT_EXECUTABLE}")
            set(QT_DEPLOY_TOOL_FOUND FALSE)
        endif()
    else()
        message(WARNING "Qt6::qmake target not found, cannot locate windeployqt")
        set(QT_DEPLOY_TOOL_FOUND FALSE)
    endif()
elseif(APPLE)
    # Find macdeployqt for macOS deployment
    get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)

    if(QT_QMAKE_EXECUTABLE)
        get_filename_component(QT_MACDEPLOYQT_EXECUTABLE ${QT_QMAKE_EXECUTABLE} PATH)
        set(QT_MACDEPLOYQT_EXECUTABLE "${QT_MACDEPLOYQT_EXECUTABLE}/macdeployqt")

        if(EXISTS ${QT_MACDEPLOYQT_EXECUTABLE})
            message(STATUS "Found macdeployqt: ${QT_MACDEPLOYQT_EXECUTABLE}")
            set(QT_DEPLOY_TOOL_FOUND TRUE)
        else()
            message(WARNING "macdeployqt not found at: ${QT_MACDEPLOYQT_EXECUTABLE}")
            set(QT_DEPLOY_TOOL_FOUND FALSE)
        endif()
    else()
        message(WARNING "Qt6::qmake target not found, cannot locate macdeployqt")
        set(QT_DEPLOY_TOOL_FOUND FALSE)
    endif()
else()
    message(STATUS "Qt deployment tools not needed on this platform")
    set(QT_DEPLOY_TOOL_FOUND FALSE)
endif()

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
# Add Windows icon resource only on Windows
set(app_icon_resource_windows "")

if(WIN32)
    set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/icons/Download Sorter.rc")
endif()

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${app_icon_resource_windows} "./icons/icons.qrc")

# Include the eUpdater subproject or find it as system package
find_package(eUpdater QUIET)

if(eUpdater_FOUND)
    message(STATUS "Using system-installed eUpdater")

# No build dependency needed - eUpdater is already installed
else()
    message(STATUS "Using eUpdater as subproject")
    add_subdirectory(${CMAKE_SOURCE_DIR}/../eUpdater ${CMAKE_BINARY_DIR}/eUpdater)

    # Ensure eUpdater builds before the main app (and is available for install)
    add_dependencies(${PROJECT_NAME} eUpdater)
endif()

# include_directories("src/Utils" "Utils/")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "DownloadSorter")

# Set GUI subsystem for all Windows builds to eliminate console window
if(WIN32)
    set_property(TARGET ${PROJECT_NAME} PROPERTY WIN32_EXECUTABLE true)
endif()

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# libraries
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core)

# Expose version to the application for fallback when manifest.json isn't available at runtime
target_compile_definitions(${PROJECT_NAME} PRIVATE APP_VERSION="${PROJECT_VERSION}")

# ━━━━━━━━━━━━━━━━━━━━━━━━━ Installation ━━━━━━━━━━━━━━━━━━━━━━━━━
# Local install directory inside the project
set(INSTALL_DIR "${CMAKE_SOURCE_DIR}/../install")

# Keep default install() using this prefix (overridden by --prefix below if needed)
set(CMAKE_INSTALL_PREFIX "${INSTALL_DIR}" CACHE PATH "Install prefix" FORCE)

# Choose platform plugins to deploy per OS
if(WIN32)
    set(QT_DEPLOY_PLUGINS_LIST "platforms;qwindows;iconengines;qsvgicon;imageformats;qico")
    set(QT_DEPLOY_EXCLUDE_LIBS_LIST "Qt6Network;D3DCompiler_47;dxcompiler;dxil;opengl32sw")
else()
    set(QT_DEPLOY_PLUGINS_LIST "platforms;qxcb")
    set(QT_DEPLOY_EXCLUDE_LIBS_LIST "Qt6Network")
endif()

# Install Scripts (Installed in Bin directory for win and linux)
if(APPLE)
    set(executable_path "$<TARGET_FILE_NAME:${PROJECT_NAME}>.app")
elseif(WIN32)
    set(executable_path "$<TARGET_FILE_NAME:${PROJECT_NAME}>")
else()
    set(executable_path "$<TARGET_FILE_NAME:${PROJECT_NAME}>")
endif()

# Harder compression for Qt resources
set(CMAKE_AUTORCC_OPTIONS --compress 9 --threshold 1)

# Size optimization for Release/MinSizeRel builds
if(MINGW OR UNIX)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release,MinSizeRel>:-Os -ffunction-sections -fdata-sections>
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release,MinSizeRel>:-Wl,--gc-sections -s>
    )
endif()

# MinGW size optimization for Release/MinSizeRel (legacy - keep for backward compatibility)
if(MINGW)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release,MinSizeRel>:-Os -ffunction-sections -fdata-sections>
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release,MinSizeRel>:-Wl,--gc-sections -s>
    )
endif()

# Ensure RPATH allows loading libraries next to the executable on UNIX
if(UNIX AND NOT APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib"
        BUILD_WITH_INSTALL_RPATH OFF
    )
endif()

# Create the runtime deploy script only on platforms where Qt provides deploy tools
if(WIN32 OR APPLE)
    if(QT_DEPLOY_TOOL_FOUND)
        set(deploy_script "${CMAKE_CURRENT_BINARY_DIR}/deploy_${PROJECT_NAME}.cmake")
        set(MAIN_BASENAME "DownloadSorter")
        set(UPDATER_BASENAME "eUpdater")
        configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/DeployRuntime.cmake.in"
            "${deploy_script}"
            @ONLY
        )
        message(STATUS "Qt deployment script configured: ${deploy_script}")
    else()
        message(STATUS "Skipping Qt deploy script (Qt deployment tool not found)")
    endif()
else()
    message(STATUS "Qt deployment not needed on this platform")
endif()

# Variables for deploy template substitution
set(QT_DEPLOY_EXCLUDE_LIBS_LIST "${QT_DEPLOY_EXCLUDE_LIBS_LIST}")
set(QT_DEPLOY_PLUGINS_LIST "${QT_DEPLOY_PLUGINS_LIST}")
set(executable_path "${executable_path}")

# Install the app (goes to ${INSTALL_DIR}) and deploy Qt runtime deps
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)

# Install eUpdater - either from subproject or system installation
if(eUpdater_FOUND)
    # Copy system-installed eUpdater to local install directory (root, not bin/)
    find_program(EUPDATER_EXECUTABLE eUpdater)

    if(EUPDATER_EXECUTABLE)
        install(PROGRAMS ${EUPDATER_EXECUTABLE} DESTINATION .)
        message(STATUS "Will install system eUpdater from: ${EUPDATER_EXECUTABLE}")
    else()
        message(WARNING "Could not find eUpdater executable to install")
    endif()
endif()

if(WIN32 OR APPLE)
    if(QT_DEPLOY_TOOL_FOUND)
        # Pass CMAKE_INSTALL_PREFIX at install time
        message(STATUS "Will install Qt deploy script: ${deploy_script}")
        install(SCRIPT ${deploy_script})
    else()
        message(STATUS "Skipping Qt deploy script (Qt deployment tool not found)")
    endif()
endif()

# Also install manifest.json so eUpdater and app can read it at runtime
install(FILES "${CMAKE_SOURCE_DIR}/../manifest.json" DESTINATION .)

install(PROGRAMS "${CMAKE_SOURCE_DIR}/../install.sh" DESTINATION .)

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/icons/Download Sorter.png"
    DESTINATION .
    RENAME "DownloadSorter.png"
)

# Prune optional/banned bits that may remain or from previous installs using a script file
install(SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/PruneInstall.cmake")

# Convenience target: copy to install and run deploy (strip in Release/MinSizeRel)
# Only use --strip on non-MSVC toolchains
set(INSTALL_STRIP_FLAG "")

if(NOT MSVC)
    set(INSTALL_STRIP_FLAG "$<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:--strip>")
endif()

# Build-time config argument for multi-config generators (Visual Studio, Xcode)
set(INSTALL_CONFIG_ARGS "")

if(CMAKE_CONFIGURATION_TYPES)
    set(INSTALL_CONFIG_ARGS --config $<CONFIG>)
endif()

add_custom_target(install_local
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${INSTALL_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${INSTALL_DIR}"
    COMMAND ${CMAKE_COMMAND} --install "${CMAKE_BINARY_DIR}" --prefix "${INSTALL_DIR}" ${INSTALL_CONFIG_ARGS} ${INSTALL_STRIP_FLAG}
    DEPENDS ${PROJECT_NAME} $<$<NOT:$<BOOL:${eUpdater_FOUND}>>:eUpdater>
    COMMENT "Installing to ${INSTALL_DIR} and deploying Qt runtime dependencies"
)

# Post-install size optimization (attach after install_local target exists)
# Always add commands; they are fast and safe. Optionally guarded by config using generator expressions if needed.
if(UNIX)
    find_program(STRIP_EXECUTABLE strip)

    if(STRIP_EXECUTABLE)
        add_custom_command(TARGET install_local POST_BUILD
            COMMAND find "${INSTALL_DIR}" -type f \( -name "DownloadSorter" -o -name "*.so*" \) -print -exec ${STRIP_EXECUTABLE} --strip-unneeded {} \;
            COMMENT "Stripping debug symbols from binaries and libraries"
            VERBATIM
        )
    endif()

    find_program(UPX_EXECUTABLE upx)

    if(UPX_EXECUTABLE)
        # Compress only the main binary; avoid shell redirections that can fail on CI
        add_custom_command(TARGET install_local POST_BUILD
            COMMAND "${UPX_EXECUTABLE}" --best "${INSTALL_DIR}/DownloadSorter"
            COMMENT "Compressing binaries with UPX"
            VERBATIM
        )
    endif()
elseif(MINGW)
    find_program(STRIP_EXECUTABLE strip)

    if(STRIP_EXECUTABLE)
        add_custom_command(TARGET install_local POST_BUILD
            COMMAND ${STRIP_EXECUTABLE} --strip-unneeded "${INSTALL_DIR}/*.exe" 2>nul || echo "Strip completed"
            COMMENT "Stripping debug symbols from Windows binaries (MinGW)"
        )
    endif()

    find_program(UPX_EXECUTABLE upx)

    if(UPX_EXECUTABLE)
        add_custom_command(TARGET install_local POST_BUILD
            COMMAND ${UPX_EXECUTABLE} --best "${INSTALL_DIR}/*.exe" 2>nul || echo "UPX completed"
            COMMENT "Compressing Windows binaries with UPX (MinGW)"
        )
    endif()
endif()