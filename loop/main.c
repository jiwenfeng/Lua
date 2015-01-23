#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

static int
_loop(lua_State *L)
{
	if(!lua_istable(L, -1))
	{
		printf("table expected(got %s)\n", lua_typename(L, -1));
		return 0;
	}
	lua_pushnil(L);
	while(lua_next(L, -2))
	{
		switch(lua_type(L, -2))
		{
			case LUA_TNUMBER:
				printf("%d:", (int)lua_tonumber(L, -2));
				break;
			case LUA_TSTRING:
				printf("%s:", lua_tostring(L, -2));
				break;
		}
		switch(lua_type(L, -1))
		{
			case LUA_TNUMBER:
				printf("%d\n", (int)lua_tonumber(L, -1));
				break;
			case LUA_TSTRING:
				printf("%s\n", lua_tostring(L, -1));
			case LUA_TTABLE:
				_loop(L);
				break;
		}
		lua_pop(L, 1);
	}
	printf("\n");
	return 0;
}

static luaL_Reg libs[] = {
	{"loop", _loop},
	{NULL, NULL}
};

static int
luaL_openmylib(lua_State *L)
{
	luaL_newlib(L, libs);
	return 1;
}

int 
main()
{
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	luaL_requiref(L, "lib", luaL_openmylib, 0);
	if(0 != luaL_dofile(L, "main.lua"))
	{
		printf("%s\n", lua_tostring(L, -1));
		return 0;
	}
	lua_getglobal(L, "main");
	if(0 != lua_pcall(L, 0, 0, 0))
	{
		printf("%s\n", lua_tostring(L, -1));
		return 0;
	}
	lua_close(L);
	return 0;
}
