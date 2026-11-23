set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
set(VCPKG_OSX_ARCHITECTURES arm64)
# Lua C modules (socket, lua-utf8, lcurl, lzip) must share one LuaJIT with SimpleGraphic, and GLFW dlopens
# libEGL/libGLESv2 at runtime, so SimpleGraphic must use those same ANGLE images
if (PORT STREQUAL "luajit" OR PORT STREQUAL "angle")
    set(VCPKG_LIBRARY_LINKAGE dynamic)
endif ()
