# macOS build

Builds SimpleGraphic natively for Apple Silicon and packages it into a Path of Building `runtime/` folder, next to
the Windows files.

    git submodule update --init --recursive
    ./vcpkg/bootstrap-vcpkg.sh -disableMetrics
    cmake -S . -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release \
        -DVCPKG_OVERLAY_TRIPLETS="$PWD/triplets" \
        -DVCPKG_TARGET_TRIPLET=arm64-osx-pob -DVCPKG_HOST_TRIPLET=arm64-osx-pob
    cmake --build build -j
    macos/package.sh /path/to/PathOfBuilding/runtime
    /path/to/PathOfBuilding/runtime/PathOfBuilding

The first configure builds every vcpkg dependency, ANGLE included, and takes a while.

- `triplets/arm64-osx-pob.cmake` links LuaJIT and ANGLE dynamically. The Lua C modules must share SimpleGraphic's
  LuaJIT, and GLFW `dlopen`s `libEGL.dylib`/`libGLESv2.dylib`, which have to be the same images SimpleGraphic links.
  The host triplet is the same so LuaJIT builds natively rather than as a cross build.
- `launcher.c` is the counterpart of `Path of Building.exe`: it puts `runtime/lua` and the runtime's modules on the
  Lua search paths, re-execs once so dyld can find ANGLE in `runtime/`, and runs `../src/Launch.lua`.
- `package.sh` copies the engine, LuaJIT, ANGLE and the Lua modules (renamed to the `.so` names `require` looks for)
  into `runtime/`, leaves `@loader_path` as their only rpath and ad-hoc signs them.
