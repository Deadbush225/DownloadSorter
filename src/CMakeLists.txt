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

# Enable Qt deployment support
if(Qt6_FOUND AND (APPLE OR (WIN32 AND MSVC)))
    # Qt6 deployment support is available from Qt 6.3+
    if(Qt6_VERSION VERSION_GREATER_EQUAL "6.3.0")
        # Try multiple possible locations for Qt6DeploySupport.cmake
        set(QT_DEPLOY_SUPPORT_PATHS
            "${Qt6_DIR}/Qt6DeploySupport.cmake"
            "${Qt6_DIR}/../Qt6DeploySupport.cmake"
            "${Qt6Core_DIR}/Qt6DeploySupport.cmake"
            "${Qt6Core_DIR}/../Qt6DeploySupport.cmake"
            "${Qt6Widgets_DIR}/Qt6DeploySupport.cmake"
            "${Qt6Widgets_DIR}/../Qt6DeploySupport.cmake"
        )
        
        # Try to find the Qt installation prefix
        get_target_property(QT_CORE_LOCATION Qt6::Core LOCATION)
        if(QT_CORE_LOCATION)
            get_filename_component(QT_BIN_DIR ${QT_CORE_LOCATION} DIRECTORY)
            get_filename_component(QT_INSTALL_PREFIX ${QT_BIN_DIR} DIRECTORY)
            list(APPEND QT_DEPLOY_SUPPORT_PATHS
                "${QT_INSTALL_PREFIX}/lib/cmake/Qt6/Qt6DeploySupport.cmake"
                "${QT_INSTALL_PREFIX}/../lib/cmake/Qt6/Qt6DeploySupport.cmake"
            )
        endif()
        
        set(QT_DEPLOY_SUPPORT_INCLUDED FALSE)
        message(STATUS "Searching for Qt6DeploySupport.cmake in:")
        foreach(deploy_path ${QT_DEPLOY_SUPPORT_PATHS})
            message(STATUS "  ${deploy_path}")
            if(EXISTS "${deploy_path}" AND NOT QT_DEPLOY_SUPPORT_INCLUDED)
                include("${deploy_path}")
                set(QT_DEPLOY_SUPPORT_INCLUDED TRUE)
                set(QT_DEPLOY_SUPPORT_FILE "${deploy_path}")
                message(STATUS "Qt6 deployment support enabled from: ${deploy_path}")
                break()
            endif()
        endforeach()
        
        if(NOT QT_DEPLOY_SUPPORT_INCLUDED)
            message(WARNING "Qt6DeploySupport.cmake not found in any of these locations:")
            foreach(deploy_path ${QT_DEPLOY_SUPPORT_PATHS})
                message(WARNING "  ${deploy_path}")
            endforeach()
        else()
            # Verify that the deployment command is available
            if(COMMAND qt_deploy_runtime_dependencies)
                message(STATUS "✓ qt_deploy_runtime_dependencies command is available")
            else()
                message(WARNING "qt_deploy_runtime_dependencies command not available after including deployment support")
            endif()
        endif()
    else()
        message(WARNING "Qt deployment requires Qt 6.3 or later (found ${Qt6_VERSION})")
    endif()
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

# Include the Updater subproject
add_subdirectory(${CMAKE_SOURCE_DIR}/../Updater ${CMAKE_BINARY_DIR}/Updater)

# include_directories("src/Utils" "Utils/")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "Download Sorter")

# Set GUI subsystem for all Windows builds to eliminate console window
if(WIN32)
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

# Create the runtime deploy script only on platforms where Qt provides deploy tools (Windows MSVC/macOS). On Linux and Windows MinGW, skip.
if(APPLE OR(WIN32 AND MSVC))
    if(QT_DEPLOY_SUPPORT_INCLUDED)
        set(deploy_script "${CMAKE_CURRENT_BINARY_DIR}/deploy_${PROJECT_NAME}.cmake")
        set(MAIN_BASENAME "Download Sorter")
        set(UPDATER_BASENAME "Updater")
        # Make sure QT_DEPLOY_SUPPORT_FILE is available for template substitution
        if(NOT DEFINED QT_DEPLOY_SUPPORT_FILE)
            set(QT_DEPLOY_SUPPORT_FILE "${Qt6_DIR}/Qt6DeploySupport.cmake")
        endif()
        configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/DeployRuntime.cmake.in"
            "${deploy_script}"
            @ONLY
        )
        message(STATUS "Qt deployment script configured: ${deploy_script}")
    else()
        message(STATUS "Skipping Qt deploy script (Qt6 deployment support not available)")
    endif()
else()
    message(STATUS "Qt deployment not supported on this platform/compiler")
endif()

# Variables for deploy template substitution
set(QT_DEPLOY_EXCLUDE_LIBS_LIST "${QT_DEPLOY_EXCLUDE_LIBS_LIST}")
set(QT_DEPLOY_PLUGINS_LIST "${QT_DEPLOY_PLUGINS_LIST}")
set(executable_path "${executable_path}")

# Install the app (goes to ${INSTALL_DIR}) and deploy Qt runtime deps
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)

if(APPLE OR(WIN32 AND MSVC))
    if(QT_DEPLOY_SUPPORT_INCLUDED)
        # Pass CMAKE_INSTALL_PREFIX at install time
        message(STATUS "Will install Qt deploy script: ${deploy_script}")
        install(SCRIPT ${deploy_script})
    else()
        message(STATUS "Skipping Qt deploy script (Qt6 deployment support not available)")
    endif()
endif()

# Also install manifest.json so Updater and app can read it at runtime
install(FILES "${CMAKE_SOURCE_DIR}/../manifest.json" DESTINATION .)

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
    DEPENDS ${PROJECT_NAME}
    COMMENT "Installing to ${INSTALL_DIR} and deploying Qt runtime dependencies"
)

# Post-install size optimization (attach after install_local target exists)
# Always add commands; they are fast and safe. Optionally guarded by config using generator expressions if needed.
if(UNIX)
    find_program(STRIP_EXECUTABLE strip)

    if(STRIP_EXECUTABLE)
        add_custom_command(TARGET install_local POST_BUILD
            COMMAND find "${INSTALL_DIR}" -type f \( -name "Download Sorter" -o -name "*.so*" \) -print -exec ${STRIP_EXECUTABLE} --strip-unneeded {} \;
            COMMENT "Stripping debug symbols from binaries and libraries"
            VERBATIM
        )
    endif()

    find_program(UPX_EXECUTABLE upx)

    if(UPX_EXECUTABLE)
        # Compress only the main binary; avoid shell redirections that can fail on CI
        add_custom_command(TARGET install_local POST_BUILD
            COMMAND "${UPX_EXECUTABLE}" --best "${INSTALL_DIR}/Download Sorter"
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