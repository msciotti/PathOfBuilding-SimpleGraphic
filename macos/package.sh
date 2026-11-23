#!/bin/bash
# Copies a macOS SimpleGraphic build into a PoB runtime/ folder: the engine, the shared LuaJIT and ANGLE it links,
# the Lua C modules under the names `require` looks for, and the launcher. Every library resolves its
# dependencies through @loader_path only, so nothing loads from the build tree.
# Usage: macos/package.sh <path to PathOfBuilding/runtime>
set -euo pipefail
SG="$(cd "$(dirname "$0")/.." && pwd)"
RT="$(cd "${1:?usage: package.sh <runtime dir>}" && pwd)"
B="$SG/build"
LIB="$B/vcpkg_installed/arm64-osx-pob/lib"

clang -O2 -Wall -o "$B/PathOfBuilding" "$SG/macos/launcher.c"

cp "$B/libSimpleGraphic.dylib" "$B/PathOfBuilding" "$RT/"
cp "$LIB"/libluajit-5.1.2.*.*.dylib "$LIB/liblibEGL_angle.dylib" "$LIB/liblibGLESv2_angle.dylib" "$RT/"
cp "$B/libsocket.dylib" "$RT/socket.so"
cp "$B/liblua-utf8.dylib" "$RT/lua-utf8.so"
cp "$B/liblcurl.dylib" "$RT/lcurl.so"
cp "$B/liblzip.dylib" "$RT/lzip.so"
# GLFW dlopens ANGLE by these names; symlinks keep them the same images SimpleGraphic links
ln -sf liblibEGL_angle.dylib "$RT/libEGL.dylib"
ln -sf liblibGLESv2_angle.dylib "$RT/libGLESv2.dylib"

cd "$RT"
for f in libSimpleGraphic.dylib libluajit-5.1.2.*.*.dylib liblibEGL_angle.dylib liblibGLESv2_angle.dylib \
		socket.so lua-utf8.so lcurl.so lzip.so; do
	for r in $(otool -l "$f" | awk '/LC_RPATH/{getline; getline; print $2}'); do
		[ "$r" = "@loader_path" ] || install_name_tool -delete_rpath "$r" "$f" 2>/dev/null
	done
	otool -l "$f" | grep -q '@loader_path' || install_name_tool -add_rpath @loader_path "$f" 2>/dev/null
done
for f in libSimpleGraphic.dylib libluajit-5.1.2.*.*.dylib liblibEGL_angle.dylib liblibGLESv2_angle.dylib \
		socket.so lua-utf8.so lcurl.so lzip.so PathOfBuilding; do
	codesign -s - -f "$f" 2>/dev/null
done
echo "Packaged into $RT"
