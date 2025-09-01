# Runs at install time to remove optional/banned files from the install tree

# Common helper to safely remove matching files
function(_rm_glob)
    file(GLOB _tmp ${ARGV})

    if(_tmp)
        file(REMOVE ${_tmp})
    endif()
endfunction()

# Remove software OpenGL
_rm_glob("${CMAKE_INSTALL_PREFIX}/opengl32sw.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/Qt6OpenGL*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/Qt6OpenGLWidgets*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/Qt6ANGLE*.dll")

# Remove debug builds
_rm_glob("${CMAKE_INSTALL_PREFIX}/*d.dll")

# Remove QtNetwork (if not required at runtime)
_rm_glob("${CMAKE_INSTALL_PREFIX}/Qt6Network.dll")

# Remove ICU and DBus if present (not needed for basic widgets apps on MSVC)
_rm_glob("${CMAKE_INSTALL_PREFIX}/icudt*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/icuin*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/icuuc*.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/Qt6DBus*.dll")

# Remove plugin folders we don't want
file(REMOVE_RECURSE
    "${CMAKE_INSTALL_PREFIX}/tls"
    "${CMAKE_INSTALL_PREFIX}/networkinformation"
    "${CMAKE_INSTALL_PREFIX}/bearer"
)

# Remove image format plugins we don’t use (keep png)
file(GLOB _img_unneeded
    "${CMAKE_INSTALL_PREFIX}/imageformats/qgif*.dll"
    "${CMAKE_INSTALL_PREFIX}/imageformats/qjpeg*.dll"
    "${CMAKE_INSTALL_PREFIX}/imageformats/qsvg*.dll"
)

if(_img_unneeded)
    file(REMOVE ${_img_unneeded})
endif()

# Remove TUIO touch plugin
_rm_glob("${CMAKE_INSTALL_PREFIX}/generic/qtuiotouchplugin*.dll")

# Remove OpenSSL if it slipped in
_rm_glob(
    "${CMAKE_INSTALL_PREFIX}/libssl*.dll"
    "${CMAKE_INSTALL_PREFIX}/libcrypto*.dll"
)

# Remove D3DCompiler (ANGLE) and DirectX components if copied
_rm_glob(
    "${CMAKE_INSTALL_PREFIX}/d3dcompiler*.dll"
    "${CMAKE_INSTALL_PREFIX}/D3DCompiler*.dll"
    "${CMAKE_INSTALL_PREFIX}/dxcompiler*.dll"
    "${CMAKE_INSTALL_PREFIX}/dxil*.dll"
)

# Remove Visual C++ redistributable (should be installed separately)
_rm_glob("${CMAKE_INSTALL_PREFIX}/vc_redist*.exe")

# Additional cleanup to ensure unwanted DLLs are removed
_rm_glob("${CMAKE_INSTALL_PREFIX}/opengl32sw.dll")
_rm_glob("${CMAKE_INSTALL_PREFIX}/Qt6Network.dll")

# Remove unwanted plugin directories
file(REMOVE_RECURSE
    "${CMAKE_INSTALL_PREFIX}/tls"
    "${CMAKE_INSTALL_PREFIX}/networkinformation"
    "${CMAKE_INSTALL_PREFIX}/bearer"
    "${CMAKE_INSTALL_PREFIX}/generic"
    "${CMAKE_INSTALL_PREFIX}/translations"
)

# Final aggressive cleanup for persistent DLLs
message(STATUS "Performing final cleanup of unwanted DLLs...")

foreach(_unwanted_dll IN ITEMS
    "opengl32sw.dll"
    "d3dcompiler_47.dll"
    "dxcompiler.dll"
    "dxil.dll"
    "Qt6Network.dll"
    "vc_redist.x64.exe")
    set(_full_path "${CMAKE_INSTALL_PREFIX}/${_unwanted_dll}")

    if(EXISTS "${_full_path}")
        file(REMOVE "${_full_path}")
        message(STATUS "Removed: ${_unwanted_dll}")
    endif()
endforeach()
