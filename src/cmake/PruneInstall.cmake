# Runs at install time to remove optional/banned files from the install tree

# Common helper to safely remove matching files
function(_rm_glob)
    file(GLOB _tmp ${ARGV})

    if(_tmp)
        foreach(_file IN LISTS _tmp)
            message(STATUS "Removing: ${_file}")
            file(REMOVE "${_file}")
        endforeach()
    endif()
endfunction()

# Remove software OpenGL and graphics acceleration DLLs
_rm_glob("${CMAKE_INSTALL_PREFIX}/opengl32sw.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/bin/opengl32sw.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/Qt6OpenGL*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/bin/Qt6OpenGL*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/Qt6OpenGLWidgets*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/bin/Qt6OpenGLWidgets*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/Qt6ANGLE*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/bin/Qt6ANGLE*.dll")

# Remove debug builds
_rm_glob("${CMAKE_INSTALL_PREFIX}/*d.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/bin/*d.dll")

# Remove unused Qt modules
_rm_glob("${CMAKE_INSTALL_PREFIX}/Qt6Network.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/bin/Qt6Network.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/Qt6Concurrent.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/bin/Qt6Concurrent.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/Qt6PrintSupport.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/bin/Qt6PrintSupport.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/Qt6Multimedia*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/bin/Qt6Multimedia*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/Qt6Quick*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/bin/Qt6Quick*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/Qt6Qml*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/bin/Qt6Qml*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/Qt6Test.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/bin/Qt6Test.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/Qt6Sql.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/bin/Qt6Sql.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/Qt6Xml.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/bin/Qt6Xml.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/Qt6SerialPort.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/bin/Qt6SerialPort.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/Qt6WebEngine*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/bin/Qt6WebEngine*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/Qt6Positioning.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/bin/Qt6Positioning.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/Qt6Sensors.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/bin/Qt6Sensors.dll")

# Remove ICU and DBus if present (not needed for basic widgets apps on MSVC)
_rm_glob("${CMAKE_INSTALL_PREFIX}/icudt*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/bin/icudt*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/icuin*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/bin/icuin*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/icuuc*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/bin/icuuc*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/Qt6DBus*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/bin/Qt6DBus*.dll")

# Remove MSVC runtime DLLs (should be installed system-wide)
_rm_glob("${CMAKE_INSTALL_PREFIX}/concrt140.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/bin/concrt140.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/msvcp140*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/bin/msvcp140*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/vcruntime140*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/bin/vcruntime140*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/api-ms-*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/bin/api-ms-*.dll")

# Remove plugin folders we don't want
file(REMOVE_RECURSE
    "${CMAKE_INSTALL_PREFIX}/tls"
    "${CMAKE_INSTALL_PREFIX}/networkinformation"
    "${CMAKE_INSTALL_PREFIX}/bearer"
    "${CMAKE_INSTALL_PREFIX}/sqldrivers"
    "${CMAKE_INSTALL_PREFIX}/multimedia"
    "${CMAKE_INSTALL_PREFIX}/qmltooling"
    "${CMAKE_INSTALL_PREFIX}/quick"
    "${CMAKE_INSTALL_PREFIX}/scenegraph"
    "${CMAKE_INSTALL_PREFIX}/translations"
    "${CMAKE_INSTALL_PREFIX}/generic"
)

# Remove image format plugins we don't use (keep png)
file(GLOB _img_unneeded
    "${CMAKE_INSTALL_PREFIX}/imageformats/qgif*.dll"
    "${CMAKE_INSTALL_PREFIX}/imageformats/qjpeg*.dll"
    "${CMAKE_INSTALL_PREFIX}/imageformats/qsvg*.dll"
    "${CMAKE_INSTALL_PREFIX}/bin/imageformats/qgif*.dll"
    "${CMAKE_INSTALL_PREFIX}/bin/imageformats/qjpeg*.dll"
    "${CMAKE_INSTALL_PREFIX}/bin/imageformats/qsvg*.dll"
)

if(_img_unneeded)
    foreach(_file IN LISTS _img_unneeded)
        message(STATUS "Removing image plugin: ${_file}")
        file(REMOVE "${_file}")
    endforeach()
endif()

# Remove OpenSSL if it slipped in
_rm_glob(
    "${CMAKE_INSTALL_PREFIX}/libssl*.dll"
    "${CMAKE_INSTALL_PREFIX}/bin/libssl*.dll"
    "${CMAKE_INSTALL_PREFIX}/libcrypto*.dll"
    "${CMAKE_INSTALL_PREFIX}/bin/libcrypto*.dll"
)

# Remove D3DCompiler (ANGLE) and DirectX components if copied
_rm_glob(
    "${CMAKE_INSTALL_PREFIX}/d3dcompiler*.dll"
    "${CMAKE_INSTALL_PREFIX}/bin/d3dcompiler*.dll"
    "${CMAKE_INSTALL_PREFIX}/D3DCompiler*.dll"
    "${CMAKE_INSTALL_PREFIX}/bin/D3DCompiler*.dll"
    "${CMAKE_INSTALL_PREFIX}/dxcompiler*.dll"
    "${CMAKE_INSTALL_PREFIX}/bin/dxcompiler*.dll"
    "${CMAKE_INSTALL_PREFIX}/dxil*.dll"
    "${CMAKE_INSTALL_PREFIX}/bin/dxil*.dll"
)

# Remove MinGW runtime DLLs (not applicable on MSVC, but safe if present)
_rm_glob(
    "${CMAKE_INSTALL_PREFIX}/libgcc_s_seh-1.dll"
    "${CMAKE_INSTALL_PREFIX}/bin/libgcc_s_seh-1.dll"
    "${CMAKE_INSTALL_PREFIX}/libstdc++-6.dll"
    "${CMAKE_INSTALL_PREFIX}/bin/libstdc++-6.dll"
    "${CMAKE_INSTALL_PREFIX}/libwinpthread-1.dll"
    "${CMAKE_INSTALL_PREFIX}/bin/libwinpthread-1.dll"
)

# Remove unused Boost DLLs (keep only those used by this project)
# Download Sorter only uses basic Qt and doesn't need Boost
_rm_glob(
    "${CMAKE_INSTALL_PREFIX}/boost_*.dll"
    "${CMAKE_INSTALL_PREFIX}/bin/boost_*.dll"
    "${CMAKE_INSTALL_PREFIX}/libboost*.dll"
    "${CMAKE_INSTALL_PREFIX}/bin/libboost*.dll"
    "${CMAKE_INSTALL_PREFIX}/*boost*.so"
    "${CMAKE_INSTALL_PREFIX}/bin/*boost*.so"
)

message(STATUS "PruneInstall.cmake completed")
