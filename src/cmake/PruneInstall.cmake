# Runs at install time to remove optional/banned files from the install tree

# remove software OpenGL
file(REMOVE "${CMAKE_INSTALL_PREFIX}/bin/opengl32sw.dll")

# remove QtNetwork just in case
file(REMOVE
    "${CMAKE_INSTALL_PREFIX}/bin/Qt6Network.dll"
    "${CMAKE_INSTALL_PREFIX}/bin/Qt6Networkd.dll")

# remove plugin folders we don't want
file(REMOVE_RECURSE
    "${CMAKE_INSTALL_PREFIX}/bin/tls"
    "${CMAKE_INSTALL_PREFIX}/bin/networkinformation"
    "${CMAKE_INSTALL_PREFIX}/bin/bearer")

# remove qgif and qjpeg plugins explicitly
file(GLOB _qgif "${CMAKE_INSTALL_PREFIX}/bin/imageformats/qgif*.dll")

if(_qgif)
    file(REMOVE ${_qgif})
endif()

file(GLOB _qjpeg "${CMAKE_INSTALL_PREFIX}/bin/imageformats/qjpeg*.dll")

if(_qjpeg)
    file(REMOVE ${_qjpeg})
endif()

# remove TUIO touch plugin explicitly
file(GLOB _qtuiot "${CMAKE_INSTALL_PREFIX}/bin/generic/qtuiotouchplugin*.dll")

if(_qtuiot)
    file(REMOVE ${_qtuiot})
endif()

# remove OpenSSL if it slipped in
file(GLOB _openssl
    "${CMAKE_INSTALL_PREFIX}/bin/libssl*.dll"
    "${CMAKE_INSTALL_PREFIX}/bin/libcrypto*.dll")

if(_openssl)
    file(REMOVE ${_openssl})
endif()

# remove D3DCompiler (ANGLE) if copied
file(GLOB _d3dc
    "${CMAKE_INSTALL_PREFIX}/bin/d3dcompiler*.dll"
    "${CMAKE_INSTALL_PREFIX}/bin/D3DCompiler*.dll")

if(_d3dc)
    file(REMOVE ${_d3dc})
endif()

# remove MinGW runtime DLLs if present (for static runtime builds)
file(REMOVE
    "${CMAKE_INSTALL_PREFIX}/bin/libgcc_s_seh-1.dll"
    "${CMAKE_INSTALL_PREFIX}/bin/libstdc++-6.dll"
    "${CMAKE_INSTALL_PREFIX}/bin/libwinpthread-1.dll")
