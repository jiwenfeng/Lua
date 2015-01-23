#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

typedef struct array_tag
{
	int size;
	int array[1];
}Array;

static int newarray(lua_State *L)
{
	int size = lua_tonumber(L, -1);
	luaL_argcheck(L, 0 < size, 2, "error size");
	int nbytes = sizeof(Array) + size * sizeof(int);
	Array *a = (Array *)lua_newuserdata(L, nbytes);
	a->size = size;
	int i = 0;
	for(i = 0; i < size; i++)
	{
		a->array[i] = i;
	}
	luaL_getmetatable(L, "Metatable.array");
	lua_setmetatable(L, -2);
	return 1;
}

static int getsize(lua_State *L)
{
//	Array *a = (Array *)lua_touserdata(L, 1);
	Array *a = (Array *)luaL_checkudata(L, 1, "Metatable.array");
	luaL_argcheck(L, a, 1, "'array' expected");
	lua_pushnumber(L, a->size);
	return 1;
}

static int setvalue(lua_State *L)
{
	Array *a = (Array *)lua_touserdata(L, 1);
	luaL_argcheck(L, a, 1, "'array' expected");
	int index = lua_tonumber(L, 2);
	luaL_argcheck(L, 0 <= index && index < a->size, 2, "index out of range");
	int value = lua_tonumber(L, 3);
	a->array[index] = value;
	return 1;
}

static int getvalue(lua_State *L)
{
	Array *a = (Array *)lua_touserdata(L, 1);
	luaL_argcheck(L, a, 1, "'array' expected");
	int index = luaL_checkint(L, 2);
	luaL_argcheck(L, 0 <= index && index < a->size, 2, "index out of range");
	lua_pushnumber(L, a->array[index]);
	return 1;
}

static int getarray(lua_State *L)
{
	Array *a = lua_touserdata(L, 1);
	luaL_argcheck(L, a, 1, "'array' expected");
	int i = 0;
	lua_newtable(L);
	for(i = 0; i < a->size; i++)
	{
		lua_pushnumber(L, a->array[i]);
		lua_rawseti(L, -2, i + 1);
	}
	return 1;
}

static int array2string(lua_State *L)
{
	Array *a = (Array *)luaL_checkudata(L, 1, "Metatable.array");
	lua_pushfstring(L, "array(%d)", a->size);
	return 1;
}

static const struct luaL_Reg arrayLib_f[] = {
	{"new", newarray},
	{NULL, NULL}
};

static const struct luaL_Reg arrayLib_m[] = {
	{"__tostring", array2string},
	{"size", getsize},
	{"set", setvalue},
	{"get", getvalue},
	{"array", getarray},
	{NULL, NULL}
};

int lua_openarray(lua_State *L)
{
	luaL_newmetatable(L, "Metatable.array");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_register(L, NULL, arrayLib_m);
	luaL_register(L, "array", arrayLib_f);
	return 1;
}

int main()
{

	lua_State *L = lua_open();
	luaL_openlibs(L);
	lua_openarray(L);
	if(0 != luaL_dofile(L, "main.lua"))
	{
		printf("%s\n", (char *)lua_tostring(L, -1));
	}
	return 0;
}
