// macOS launcher for Path of Building: the counterpart of "Path of Building.exe".
// Lives in runtime/ next to libSimpleGraphic.dylib, points LuaJIT's search paths at runtime/lua and the
// runtime's native modules, then hands Launch.lua to SimpleGraphic's RunLuaFileAsWin.
#include <dlfcn.h>
#include <libgen.h>
#include <limits.h>
#include <mach-o/dyld.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef int (*RunLuaFileAsWin_t)(int argc, char** argv);

int main(int argc, char** argv)
{
	char exe[PATH_MAX], real[PATH_MAX], dir[PATH_MAX];
	uint32_t size = sizeof(exe);
	if (_NSGetExecutablePath(exe, &size) != 0 || !realpath(exe, real)) {
		fprintf(stderr, "Cannot resolve executable path\n");
		return 1;
	}
	strlcpy(dir, dirname(real), sizeof(dir));

	char buf[PATH_MAX * 3];

	// GLFW dlopens ANGLE by leaf name ("libEGL.dylib"), which dyld only resolves through its search paths; those are
	// read at process start, so re-exec once with runtime/ on the fallback path
	if (!getenv("POB_RUNTIME_DIR")) {
		const char* home = getenv("HOME");
		snprintf(buf, sizeof(buf), "%s:%s/lib:/usr/local/lib:/usr/lib", dir, home ? home : "");
		setenv("DYLD_FALLBACK_LIBRARY_PATH", buf, 1);
		setenv("POB_RUNTIME_DIR", dir, 1);
		execv(real, argv);
		fprintf(stderr, "Re-exec failed, continuing without runtime/ on the library path\n");
	}
	snprintf(buf, sizeof(buf), "%s/lua/?.lua;%s/lua/?/init.lua;;", dir, dir);
	setenv("LUA_PATH", buf, 0);
	snprintf(buf, sizeof(buf), "%s/?.so;%s/?.dylib;;", dir, dir);
	setenv("LUA_CPATH", buf, 0);

	// Script: first argument, else the repository's src/Launch.lua (runtime/ is its sibling), else ./Launch.lua
	char script[PATH_MAX];
	int firstArg = 1;
	if (argc > 1 && strstr(argv[1], ".lua")) {
		strlcpy(script, argv[1], sizeof(script));
		firstArg = 2;
	} else {
		snprintf(script, sizeof(script), "%s/../src/Launch.lua", dir);
		if (access(script, R_OK) != 0) snprintf(script, sizeof(script), "%s/Launch.lua", dir);
	}

	snprintf(buf, sizeof(buf), "%s/libSimpleGraphic.dylib", dir);
	void* lib = dlopen(buf, RTLD_NOW | RTLD_LOCAL);
	if (!lib) {
		fprintf(stderr, "Cannot load %s: %s\n", buf, dlerror());
		return 1;
	}
	RunLuaFileAsWin_t run = (RunLuaFileAsWin_t)dlsym(lib, "RunLuaFileAsWin");
	if (!run) {
		fprintf(stderr, "RunLuaFileAsWin not found: %s\n", dlerror());
		return 1;
	}

	// SimpleGraphic takes the script as argv[0] and passes the rest through to Lua
	int n = argc - firstArg + 1;
	char** args = calloc(n + 1, sizeof(char*));
	args[0] = script;
	for (int i = firstArg; i < argc; i++) args[i - firstArg + 1] = argv[i];
	return run(n, args);
}
