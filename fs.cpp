#include "xlua.h"

#include <filesystem>
#include <cstdlib>

#define luaFS "fs"

typedef int (*lua_CFunction) (lua_State* L);

namespace fs = std::filesystem;

static int is_directory(lua_State* L) { // is_directory(path: string) -> bool
	const char* path = luaL_checkstring(L, 1);
	lua_pushboolean(L, fs::is_directory(path));
	 return 1;
}

static int is_file(lua_State* L) { //  is_file(path: string) -> bool
	const char* path = luaL_checkstring(L, 1);
	lua_pushboolean(L, fs::is_regular_file(path));
	return 1;
}

static int file_exists(lua_State* L) { // file_exists(path: string) -> bool
	const char* path = luaL_checkstring(L, 1);
	struct stat buffer;
	lua_pushboolean(L,stat(path, &buffer) == 0);
	return 1;
}

static int listdir(lua_State* L) { // listdir(path: string, type: integer) -> table[integer, string]
	const char* dir_path = luaL_checkstring(L, 1);
	int type = 0;
	if (lua_gettop(L) == 2) {
		type = luaL_checknumber(L, 2);
	}

	int index = 1; // start from 1
	if (!(fs::is_directory(dir_path))) {
		return luaL_error(L, "expected a vaild dir path (got", dir_path, ")");
	}
	lua_newtable(L);
	for (const auto& entry : fs::directory_iterator(dir_path)) {
		if (type == 1 && !(fs::is_directory(entry))) {
			continue;
		}
		else if (type == 2 && !(fs::is_regular_file(entry))) {
			continue;
		}
		lua_pushnumber(L, index); // push the index
		lua_pushstring(L, entry.path().string().c_str()); // Push the entry path
		lua_settable(L, -3); // set the entry in the table
		index++;
	}
	return 1;
}

static inline void registerCfunction(lua_State* L, lua_CFunction func, const char* name, int stack = -2) {
	int base = lua_gettop(L);
	if (base <= 0)
		return;
	lua_pushcfunction(L, func);
	lua_setfield(L, stack, name);
}

extern "C" __declspec(dllexport) int luaopen_fs (lua_State *L) {
	lua_newtable(L);
	lua_setglobal(L, luaFS);
	lua_getglobal(L, luaFS);
	registerCfunction(L, is_directory, "is_directory");
	registerCfunction(L, is_file, "is_file");
	registerCfunction(L, file_exists, "file_exists");
	registerCfunction(L, listdir, "listdir");

	// values
	lua_pushnumber(L, 0);
	lua_setfield(L, -2, "BOTH");

	lua_pushnumber(L, 1);
	lua_setfield(L, -2, "DIRS");

	lua_pushnumber(L, 2);
	lua_setfield(L, -2, "FILES");

	return 0;
}
