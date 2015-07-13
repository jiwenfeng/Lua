#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <dirent.h>
#include <errno.h>
static int dir_iter(lua_State *L)
{
	DIR * d = *(DIR **)lua_touserdata(L, lua_upvalueindex(1));
	struct dirent *entry;
	if((entry = readdir(d)))
	{
		lua_pushstring(L, entry->d_name);
		return 1;
	}
	return 0;
}

static int dir_gc(lua_State *L)
{
	DIR *d = *(DIR **)lua_touserdata(L, 1);
	if(d)
	{
		closedir(d);
	}
	return 0;
}

static int l_dir(lua_State *L)
{
	const char *path = luaL_checkstring(L, 1);
	DIR ** d = (DIR **) lua_newuserdata(L, sizeof(DIR *));
	luaL_getmetatable(L, "LuaDir.Dir");
	lua_setmetatable(L, -2);
	*d = opendir(path);
	if(!d)
	{
		luaL_error(L, "can not open%s:%s", path, strerror(errno));
	}
	lua_pushcclosure(L, dir_iter, 1);
	return 1;
}

int lua_opendir(lua_State *L)
{
	luaL_newmetatable(L, "LuaDir.Dir");
	lua_pushstring(L, "__gc");
	lua_pushcfunction(L, dir_gc);
	lua_settable(L, -3);

	lua_pushcfunction(L, l_dir);
	lua_setglobal(L, "dir");
	return 0;
}


int main()
{
	lua_State *L = luaL_newstate();
//	lua_State *L = lua_open();
	luaL_openlibs(L);
	lua_opendir(L);
	if(0 != luaL_dofile(L, "main.lua"))
	{
		printf("%s\n", lua_tostring(L, -1));
	}
	return 0;
}
