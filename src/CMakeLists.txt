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

find_package(Boost 1.81.0)

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

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET ${PROJECT_NAME} PROPERTY WIN32_EXECUTABLE true)
endif()

# target_include_directories(${PROJECT_NAME} PRIVATE "src/Utils/")
# add_executable(Testsd ${SOURCE_FILES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# libraries
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core)

if(Boost_FOUND)
    message("Boost Found...")
    target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_LIBRARIES})
endif()

# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)

# Local install directory inside the project
set(INSTALL_DIR "${CMAKE_SOURCE_DIR}/install")

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

# Ensure QtNetwork isn’t deployed
set(QT_DEPLOY_EXCLUDE_MODULES "Network")

set(QT_DEPLOY_EXCLUDE_PLUGINS
    "sqldrivers;*;tls;*;mediaservice;*;position;*;webengine;*;networkinformation;*;qmltooling;*;sensors;*;geoservices;*;texttospeech;*;virtualkeyboard;*;platforminputcontexts;*;iconengines;*;"
)

set(deploy_script "${CMAKE_CURRENT_BINARY_DIR}/deploy_${PROJECT_NAME}.cmake")
file(GENERATE OUTPUT ${deploy_script} CONTENT "
# Hard excludes and explicit whitelist so deploy won't bring optional stuff
set(QT_DEPLOY_EXCLUDE_MODULES \"Network;NetworkInformation;NetworkAuth\")
set(QT_DEPLOY_EXCLUDE_PLUGINS \"tls;*;networkinformation;*;bearer;*;imageformats;qgif;qjpeg;qsvg;qsvgicon;generic;qtuiotouchplugin;\")
set(QT_DEPLOY_EXCLUDE_LIBRARIES \"Qt6Network;D3DCompiler_47\")
set(QT_DEPLOY_PLUGINS
    \"platforms;qwindows\")
include(\"${QT_DEPLOY_SUPPORT}\")
qt_deploy_runtime_dependencies(
    EXECUTABLE \"${executable_path}\"
    PLUGINS_DIR bin
    NO_TRANSLATIONS
)")

# Install the app (goes to ${INSTALL_DIR}/bin) and deploy Qt runtime deps
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(SCRIPT ${deploy_script})

# Prune optional/banned bits that may remain or from previous installs
install(CODE "
# remove software OpenGL
file(REMOVE \"\${CMAKE_INSTALL_PREFIX}/bin/opengl32sw.dll\")
# remove QtNetwork just in case
file(REMOVE
  \"\${CMAKE_INSTALL_PREFIX}/bin/Qt6Network.dll\"
  \"\${CMAKE_INSTALL_PREFIX}/bin/Qt6Networkd.dll\")
# remove plugin folders we don't want
file(REMOVE_RECURSE
  \"\${CMAKE_INSTALL_PREFIX}/bin/tls\"
  \"\${CMAKE_INSTALL_PREFIX}/bin/networkinformation\"
  \"\${CMAKE_INSTALL_PREFIX}/bin/bearer\")

# remove qgif plugin explicitly
file(GLOB _qgif \"\${CMAKE_INSTALL_PREFIX}/bin/imageformats/qgif*.dll\")
if(_qgif)
  file(REMOVE \${_qgif})
endif()
file(GLOB _qgif \"\${CMAKE_INSTALL_PREFIX}/bin/imageformats/qjpeg*.dll\")
if(_qgif)
  file(REMOVE \${_qgif})
endif()
# remove TUIO touch plugin explicitly
file(GLOB _qtuiot \"\${CMAKE_INSTALL_PREFIX}/bin/generic/qtuiotouchplugin*.dll\")
if(_qtuiot)
  file(REMOVE \${_qtuiot})
endif()
# remove OpenSSL if it slipped in
file(GLOB _openssl
  \"\${CMAKE_INSTALL_PREFIX}/bin/libssl*.dll\"
  \"\${CMAKE_INSTALL_PREFIX}/bin/libcrypto*.dll\")
if(_openssl)
  file(REMOVE \${_openssl})
endif()
# remove D3DCompiler (ANGLE) if copied
file(GLOB _d3dc
  \"\${CMAKE_INSTALL_PREFIX}/bin/d3dcompiler*.dll\"
  \"\${CMAKE_INSTALL_PREFIX}/bin/D3DCompiler*.dll\")
if(_d3dc)
  file(REMOVE \${_d3dc})
endif()
# remove MinGW runtime DLLs if present (we link them statically)
file(REMOVE
  \"\${CMAKE_INSTALL_PREFIX}/bin/libgcc_s_seh-1.dll\"
  \"\${CMAKE_INSTALL_PREFIX}/bin/libstdc++-6.dll\"
  \"\${CMAKE_INSTALL_PREFIX}/bin/libwinpthread-1.dll\")
")

# Convenience target: copy to install and run deploy (strip in Release/MinSizeRel)
add_custom_target(install_local
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${INSTALL_DIR}/bin"
    COMMAND ${CMAKE_COMMAND} --install "${CMAKE_BINARY_DIR}" --prefix "${INSTALL_DIR}" $<$<BOOL:${CMAKE_CONFIGURATION_TYPES}>:--config> $<$<BOOL:${CMAKE_CONFIGURATION_TYPES}>:$<CONFIG>> $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:--strip>
    DEPENDS ${PROJECT_NAME}
    COMMENT "Installing to ${INSTALL_DIR} and deploying Qt runtime dependencies"
)
