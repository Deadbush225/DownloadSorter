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
    # Locate QtDeploySupport cmake module from the found Qt6 package
    # Qt 6 exports a variable Qt6_DIR that contains the CMake package dir
    if(NOT DEFINED Qt6_DIR)
        message(FATAL_ERROR "Qt6_DIR is not defined; ensure CMAKE_PREFIX_PATH points to the Qt installation")
    endif()

    message(STATUS "Qt6_DIR: ${Qt6_DIR}")
    message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

    # Try typical relative path to Qt6DeploySupport.cmake
    set(_QT_DEPLOY_SUPPORT_CANDIDATES
        "${Qt6_DIR}/Qt6DeploySupport.cmake"
        "${Qt6_DIR}/../Qt6DeploySupport.cmake"
        "${Qt6_DIR}/../../Qt6DeploySupport.cmake"
        "${Qt6_DIR}/../../../lib/cmake/Qt6/Qt6DeploySupport.cmake"
        "${Qt6_DIR}/../../lib/cmake/Qt6/Qt6DeploySupport.cmake"
        "${Qt6_DIR}/../lib/cmake/Qt6/Qt6DeploySupport.cmake"
        "${Qt6_DIR}/lib/cmake/Qt6/Qt6DeploySupport.cmake"
        "${CMAKE_PREFIX_PATH}/lib/cmake/Qt6/Qt6DeploySupport.cmake"
        "${CMAKE_PREFIX_PATH}/Qt6DeploySupport.cmake"
    )
    set(QT_DEPLOY_SUPPORT_FILE "")

    foreach(p IN LISTS _QT_DEPLOY_SUPPORT_CANDIDATES)
        message(STATUS "Checking for Qt6DeploySupport.cmake at: ${p}")
        if(EXISTS "${p}")
            set(QT_DEPLOY_SUPPORT_FILE "${p}")
            message(STATUS "Found Qt6DeploySupport.cmake at: ${p}")
            break()
        endif()
    endforeach()

    if(NOT QT_DEPLOY_SUPPORT_FILE)
        message(WARNING "Qt6DeploySupport.cmake not found; skipping Qt CMake deploy. Installer will rely on windeployqt fallback.")
        message(STATUS "Searched in these locations:")
        foreach(p IN LISTS _QT_DEPLOY_SUPPORT_CANDIDATES)
            message(STATUS "  - ${p}")
        endforeach()
    else()
        message(STATUS "Using Qt deploy support file: ${QT_DEPLOY_SUPPORT_FILE}")
    endif()

    set(deploy_script "${CMAKE_CURRENT_BINARY_DIR}/deploy_${PROJECT_NAME}.cmake")
    set(MAIN_BASENAME "Download Sorter")
    set(UPDATER_BASENAME "Updater")
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/DeployRuntime.cmake.in"
        "${deploy_script}"
        @ONLY
    )
endif()

# Variables for deploy template substitution
set(QT_DEPLOY_EXCLUDE_LIBS_LIST "${QT_DEPLOY_EXCLUDE_LIBS_LIST}")
set(QT_DEPLOY_PLUGINS_LIST "${QT_DEPLOY_PLUGINS_LIST}")
set(executable_path "${executable_path}")

# Install the app (goes to ${INSTALL_DIR}) and deploy Qt runtime deps
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)

if(APPLE OR(WIN32 AND MSVC))
    if(QT_DEPLOY_SUPPORT_FILE)
        # Pass CMAKE_INSTALL_PREFIX at install time
        message(STATUS "Will install Qt deploy script: ${deploy_script}")
        install(SCRIPT ${deploy_script})
    else()
        message(STATUS "Skipping Qt deploy script (Qt6DeploySupport.cmake not found)")
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