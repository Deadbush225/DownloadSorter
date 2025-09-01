cmake_minimum_required(VERSION 3.16)
project(DownloadSorter VERSION 0.1.0)

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

# include_directories("src/Utils" "Utils/")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "Download Sorter")

# Set GUI subsystem only for Windows release builds
if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Release")
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
    set(QT_DEPLOY_PLUGINS_LIST "platforms;qwindows")
    set(QT_DEPLOY_EXCLUDE_LIBS_LIST "Qt6Network;D3DCompiler_47")
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

# Create the runtime deploy script, that gathers all the dependencies. Equivalent of "windeployqt", but with filters
# Only used on platforms where Qt provides deploy tools (Windows/macOS). On Linux, skip this step.
if(WIN32 OR APPLE)
    set(deploy_script "${CMAKE_CURRENT_BINARY_DIR}/deploy_${PROJECT_NAME}.cmake")
    file(GENERATE OUTPUT "${deploy_script}" CONTENT "
set(QT_DEPLOY_BIN_DIR ".")
set(QT_DEPLOY_LIB_DIR ".")
set(QT_DEPLOY_PLUGINS_DIR ".")  # optional when passing PLUGINS_DIR

# This file is generated at generate-time and executed at install time
set(QT_DEPLOY_EXCLUDE_MODULES \"Network;NetworkInformation;NetworkAuth\")
set(QT_DEPLOY_EXCLUDE_PLUGINS \"tls;*;networkinformation;*;bearer;*;imageformats;qgif;qjpeg;qsvg;qsvgicon;generic;qtuiotouchplugin;\")
set(QT_DEPLOY_EXCLUDE_LIBRARIES \"${QT_DEPLOY_EXCLUDE_LIBS_LIST}\")
set(QT_DEPLOY_PLUGINS \"${QT_DEPLOY_PLUGINS_LIST}\")
include(\"${QT_DEPLOY_SUPPORT}\")
qt_deploy_runtime_dependencies(
    EXECUTABLE \"${executable_path}\"
    PLUGINS_DIR .
    NO_TRANSLATIONS
)
")
endif()

# Install the app (goes to ${INSTALL_DIR}/bin) and deploy Qt runtime deps
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)
if(WIN32 OR APPLE)
    install(SCRIPT ${deploy_script})
endif()

# Prune optional/banned bits that may remain or from previous installs using a script file
install(SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/PruneInstall.cmake")

# Convenience target: copy to install and run deploy (strip in Release/MinSizeRel)
add_custom_target(install_local
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${INSTALL_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${INSTALL_DIR}"
    COMMAND ${CMAKE_COMMAND} --install "${CMAKE_BINARY_DIR}" --prefix "${INSTALL_DIR}" $<$<BOOL:${CMAKE_CONFIGURATION_TYPES}>:--config> $<$<BOOL:${CMAKE_CONFIGURATION_TYPES}>:$<CONFIG>> $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:--strip>
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
        add_custom_command(TARGET install_local POST_BUILD
            COMMAND bash -c "cd '${INSTALL_DIR}' && ${UPX_EXECUTABLE} --best 'Download Sorter' 2>/dev/null || true"
            COMMAND bash -c "cd '${INSTALL_DIR}' && ${UPX_EXECUTABLE} --best libQt6*.so.* 2>/dev/null || true"
            COMMENT "Compressing binaries with UPX"
        )
    endif()
elseif(WIN32)
    find_program(STRIP_EXECUTABLE strip)

    if(STRIP_EXECUTABLE)
        add_custom_command(TARGET install_local POST_BUILD
            COMMAND ${STRIP_EXECUTABLE} --strip-unneeded "${INSTALL_DIR}/*.exe" 2>nul || echo "Strip completed"
            COMMENT "Stripping debug symbols from Windows binaries"
        )
    endif()

    find_program(UPX_EXECUTABLE upx)

    if(UPX_EXECUTABLE)
        add_custom_command(TARGET install_local POST_BUILD
            COMMAND ${UPX_EXECUTABLE} --best "${INSTALL_DIR}/*.exe" 2>nul || echo "UPX completed"
            COMMENT "Compressing Windows binaries with UPX"
        )
    endif()
endif()