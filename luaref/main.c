#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int ref_func1, ref_func2, ref_func3;

static int test_func(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TFUNCTION);
	luaL_checktype(L, 2, LUA_TFUNCTION);
	luaL_checktype(L, 3, LUA_TFUNCTION);
	ref_func3 = lua_ref(L, LUA_REGISTRYINDEX);
	ref_func2 = lua_ref(L, LUA_REGISTRYINDEX);
	ref_func1 = lua_ref(L, LUA_REGISTRYINDEX);
	return 0;
}

static int traceback(lua_State *L)
{
	lua_getfield(L, LUA_GLOBALSINDEX, "debug");
	if(!lua_istable(L, -1))
	{
		lua_pop(L, 1);
		return 0;
	}
	lua_getfield(L, -1, "traceback");
	if(!lua_isfunction(L, -1))
	{
		lua_pop(L, 2);
		return 0;
	}
	lua_pushvalue(L, 1);
	if(0 != lua_pcall(L, 1, 1, 0))
	{
		printf("trace back error!\n");
	}
	return 0;
}

int main()
{
	lua_State *L = lua_open();
	luaL_openlibs(L);
	lua_pushcfunction(L, test_func);
	lua_setglobal(L, "TT");
	if(0 != luaL_dofile(L, "main.lua"))
	{
		printf("%s\n", (char *)lua_tostring(L, -1));
		return 0;
	}
	lua_getref(L, ref_func3);
	int base = lua_gettop(L);
	if(lua_isfunction(L, -1))
	{
		lua_pushnumber(L, 3);
		lua_pushnumber(L, 2);
		if(0 != lua_pcall(L, 2, 0, 0))
		{
			printf("call failed:%s\n", (char *)lua_tostring(L, -1));
		}
	}
	else
	{
		printf("not a function\n");
	}
	return 0;
}
