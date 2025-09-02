#!/usr/bin/env cmake -P

# Test script to check Qt6 deployment support
find_package(Qt6 REQUIRED COMPONENTS Core)

message(STATUS "Qt6_VERSION: ${Qt6_VERSION}")
message(STATUS "Qt6_DIR: ${Qt6_DIR}")

# Check for Qt6DeploySupport.cmake in standard location
set(DEPLOY_SUPPORT_PATH "${Qt6_DIR}/Qt6DeploySupport.cmake")
if(EXISTS "${DEPLOY_SUPPORT_PATH}")
    message(STATUS "✓ Found Qt6DeploySupport.cmake at: ${DEPLOY_SUPPORT_PATH}")
    include("${DEPLOY_SUPPORT_PATH}")
    
    # Test if qt_deploy_runtime_dependencies is available
    if(COMMAND qt_deploy_runtime_dependencies)
        message(STATUS "✓ qt_deploy_runtime_dependencies command is available")
    else()
        message(WARNING "✗ qt_deploy_runtime_dependencies command not available after include")
    endif()
else()
    message(WARNING "✗ Qt6DeploySupport.cmake not found at: ${DEPLOY_SUPPORT_PATH}")
    
    # Search for it in other locations
    file(GLOB_RECURSE DEPLOY_FILES "${Qt6_DIR}/../**/Qt6DeploySupport.cmake")
    if(DEPLOY_FILES)
        message(STATUS "Found Qt6DeploySupport.cmake files:")
        foreach(FILE IN LISTS DEPLOY_FILES)
            message(STATUS "  - ${FILE}")
        endforeach()
    else()
        message(STATUS "No Qt6DeploySupport.cmake files found in Qt installation")
    endif()
endif()
