# Runs at install time to remove optional/banned files from the install tree

# remove software OpenGL
file(REMOVE "${CMAKE_INSTALL_PREFIX}/opengl32sw.dll")

# remove QtNetwork just in case
file(REMOVE
    "${CMAKE_INSTALL_PREFIX}/Qt6Network.dll"
    "${CMAKE_INSTALL_PREFIX}/Qt6Networkd.dll")

# remove plugin folders we don't want
file(REMOVE_RECURSE
    "${CMAKE_INSTALL_PREFIX}/tls"
    "${CMAKE_INSTALL_PREFIX}/networkinformation"
    "${CMAKE_INSTALL_PREFIX}/bearer")

# remove qgif and qjpeg plugins explicitly
file(GLOB _qgif "${CMAKE_INSTALL_PREFIX}/imageformats/qgif*.dll")

if(_qgif)
    file(REMOVE ${_qgif})
endif()

file(GLOB _qjpeg "${CMAKE_INSTALL_PREFIX}/imageformats/qjpeg*.dll")

if(_qjpeg)
    file(REMOVE ${_qjpeg})
endif()

# remove TUIO touch plugin explicitly
file(GLOB _qtuiot "${CMAKE_INSTALL_PREFIX}/generic/qtuiotouchplugin*.dll")

if(_qtuiot)
    file(REMOVE ${_qtuiot})
endif()

# remove OpenSSL if it slipped in
file(GLOB _openssl
    "${CMAKE_INSTALL_PREFIX}/libssl*.dll"
    "${CMAKE_INSTALL_PREFIX}/libcrypto*.dll")

if(_openssl)
    file(REMOVE ${_openssl})
endif()

# remove D3DCompiler (ANGLE) if copied
file(GLOB _d3dc
    "${CMAKE_INSTALL_PREFIX}/d3dcompiler*.dll"
    "${CMAKE_INSTALL_PREFIX}/D3DCompiler*.dll")

if(_d3dc)
    file(REMOVE ${_d3dc})
endif()

# remove MinGW runtime DLLs if present (for static runtime builds)
file(REMOVE
    "${CMAKE_INSTALL_PREFIX}/libgcc_s_seh-1.dll"
    "${CMAKE_INSTALL_PREFIX}/libstdc++-6.dll"
    "${CMAKE_INSTALL_PREFIX}/libwinpthread-1.dll")
