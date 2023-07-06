cmake_minimum_required(VERSION 3.16)
project(DownloadSorter VERSION 0.1.0)

# file(GLOB_RECURSE HELPER_FILES "/src/HELPER/*.cpp")
# file(GLOB_RECURSE INCLUDE_FILES "*.h")
# CMAKE_PREFIX_PATH="C:/Qt/6.4.2/mingw_64"

# Find includes in the build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Turn on automatic invocation of the MOC, UIC & RCC
set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(Boost_USE_STATIC_LIBS OFF)

set(PROJECT_NAME DownloadSorter)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Core)
qt_standard_project_setup()

find_package(Boost 1.81.0)

file(GLOB_RECURSE DownloadSorter "DownloadSorter/*.cpp")

set(SOURCE_FILES
    ${DownloadSorter}
    main.cpp
    # DownloadSorter/subclass.cpp
    # DashboardWidget.cpp

    # ModQMap.cpp
    # ModQComboBox.cpp
    # FixedLabel.cpp
)

# add_executable(PrintingRates main.cpp DashboardWidget.cpp)
#---
add_executable(${PROJECT_NAME} ${SOURCE_FILES} icons.qrc)
# include_directories("src/Utils" "Utils/")

# target_include_directories(${PROJECT_NAME} PRIVATE "src/Utils/")
# add_executable(Testsd ${SOURCE_FILES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)


#---
target_link_libraries(DownloadSorter PRIVATE Qt6::Widgets)
target_link_libraries(DownloadSorter PRIVATE Qt6::Core)

# target_link_libraries(Testsd PRIVATE Qt6::Widgets)
# target_link_libraries(Testsd PRIVATE Qt6::Core)
if(Boost_FOUND)
    message("Boost Found...")
    target_include_directories(DownloadSorter PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(DownloadSorter PUBLIC ${Boost_LIBRARIES})
    # target_link_libraries(${PROJECT_NAME} PUBLIC Boost::boost)
endif()
# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)
