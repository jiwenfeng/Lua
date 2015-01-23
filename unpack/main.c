#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

struct head
{
	short len;
	char zip;
	int pid;
}__attribute__((packed));

static int
traceback(lua_State *L)
{
	lua_getfield(L, LUA_RIDX_GLOBALS, "debug");
	if(!lua_istable(L, -1))
	{
		return 0;
	}
	lua_getfield(L, -1, "traceback");
	if(!lua_isfunction(L, -1))
	{
		lua_pop(L, 1);
		return 0;
	}
	lua_pushvalue(L, 1);
	lua_pcall(L, 1, 0, 0);
	return 1;
}

int
main()
{
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	if(0 != luaL_dofile(L, "main.lua"))
	{
		printf("%s\n", lua_tostring(L, -1));
		return 0;
	}
	lua_pushcfunction(L, traceback);
	int err = lua_gettop(L);
	lua_getglobal(L, "main");
	if(!lua_isfunction(L, -1))
	{
		printf("%s\n", lua_tostring(L, -1));
		return 0;
	}
	struct head head;
	head.len = 10;
	head.zip = 'a';
	head.pid = 100;
	lua_pushlstring(L, (const char *)&head, sizeof(struct head));
	if(0 != lua_pcall(L, 1, 0, err))
	{
		printf("%s\n", lua_tostring(L, -1));
		return 0;
	}
	lua_close(L);
	return 0;
}
