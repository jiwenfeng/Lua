#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <mysql.h>

static int
lmysql_open(lua_State *L)
{
	MYSQL ** mysql = (MYSQL **)lua_newuserdata(L, sizeof(MYSQL *));
	mysql_library_init(0, NULL, NULL);
	*mysql = mysql_init(NULL);
	luaL_getmetatable(L, "mysql");
	lua_setmetatable(L, -2);
	return 1;
}

static int
lmysql_close(lua_State *L)
{
	MYSQL *mysql = *(MYSQL **)lua_touserdata(L, 1);
	if(NULL != mysql)
	{
		mysql_close(mysql);
		mysql_library_end();
	}
	return 0;
}

static int
lmysql_connect(lua_State *L)
{
	int narg = lua_gettop(L);
	luaL_argcheck(L, narg == 5 || narg == 6, narg, "Arguments Error");
	MYSQL *mysql = *(MYSQL **)lua_touserdata(L, 1);
	luaL_argcheck(L, mysql, 1, "`mysql' excepted ");
	const char *host = lua_tostring(L, 2);
	const char *uname = lua_tostring(L, 3);
	const char *passwd = lua_tostring(L, 4);
	const char *dbname = lua_tostring(L, 5);
	if(narg == 6)
	{
		int timeout = (int)lua_tonumber(L, 6);
		mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, (const char *)&timeout);
	}
	if(!mysql_real_connect(mysql, host, uname, passwd, dbname, 0, NULL, 0))
	{
		lua_pushnil(L);
		lua_pushstring(L, mysql_error(mysql));
		return 2;
	}
	return 1;
}

static int
lmysql_tables(lua_State *L)
{
	MYSQL *mysql = *(MYSQL **)lua_touserdata(L, 1);
	luaL_argcheck(L, mysql, 1, "`mysql' expected");
	const char *wild = lua_tostring(L, 2);
	MYSQL_RES *res = mysql_list_tables(mysql, wild);
	if(res)
	{
		MYSQL_ROW row;
		lua_newtable(L);
		int n = 1;
		while((row = mysql_fetch_row(res)))
		{
#if LUA_VERSION_NUM > 501
			lua_pushinteger(L, n++);
#else
			lua_pushnumber(L, n++);
#endif
			lua_pushstring(L, row[0]);
			lua_settable(L, -3);
		}
	}
	mysql_free_result(res);
	return 1;
}

static int
lmysql_execute(lua_State *L)
{
	MYSQL *mysql = *(MYSQL **)lua_touserdata(L, 1);
	luaL_argcheck(L, mysql, 1, "`mysql' expected");
	size_t len;
	const char *str = lua_tolstring(L, 2, &len);
	if(0 == mysql_real_query(mysql, str, len))
	{
		MYSQL_RES *res = mysql_store_result(mysql);
		if(NULL == res)
		{
			if(0 != mysql_errno(mysql))
			{
				lua_pushnil(L);
				lua_pushstring(L, mysql_error(mysql));
				return 2;
			}
#if LUA_VERSION_NUM > 501
			lua_pushinteger(L, mysql_affected_rows(mysql));
#else
			lua_pushnumber(L, mysql_affected_rows(mysql));
#endif
			return 1;
		}
		MYSQL_ROW row;
		MYSQL_FIELD *fields = mysql_fetch_fields(res);
		int nfields = mysql_num_fields(res);
		int i = 0, n = 1;
#if LUA_VERSION_NUM > 501
		lua_pushinteger(L, mysql_num_rows(res));
#else
		lua_pushnumber(L, mysql_num_rows(res));
#endif
		lua_newtable(L);
		while((row = mysql_fetch_row(res)))
		{
			unsigned long *lengths = mysql_fetch_lengths(res);
#if LUA_VERSION_NUM > 501
			lua_pushinteger(L, n++);
#else
			lua_pushnumber(L, n++);
#endif
			lua_newtable(L);
			for(i = 0; i < nfields; i++)
			{
				lua_pushstring(L, fields[i].name);
				lua_pushlstring(L, row[i], lengths[i]);
				lua_settable(L, -3);
			}
			lua_settable(L, -3);
		}
		mysql_free_result(res);
		return 2;
	}
	lua_pushnil(L);
	lua_pushstring(L, mysql_error(mysql));
	return 2;
}

static int
lmysql_tostring(lua_State *L)
{
	MYSQL *mysql = *(MYSQL **)lua_touserdata(L, 1);
	luaL_argcheck(L, mysql, 1, "`mysql' expected");
	const char *info = mysql_get_host_info(mysql);
	lua_pushstring(L, info);
	return 1;
}

static const struct luaL_Reg connection_methods[] = {
	{"connect", lmysql_connect},
	{"tostring", lmysql_tostring},
	{"execute", lmysql_execute},
	{"tables", lmysql_tables},
	{NULL, NULL},
};

static const struct luaL_Reg connection_class_methods[] = {
	{"New", lmysql_open},
	{NULL, NULL},
};

static int
mysql_register(lua_State *L)
{
	luaL_newmetatable(L, "mysql");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
#if LUA_VERSION_NUM > 501
	luaL_setfuncs(L, connection_methods, 0);
#else
	luaL_register(L, NULL, connection_methods);
#endif

	lua_pushcfunction(L, lmysql_close);
	lua_setfield(L, -2, "__gc");

#if LUA_VERSION_NUM > 501
	luaL_newlib(L, connection_class_methods);
#else
	luaL_register(L, "mysql", connection_class_methods);
#endif

	return 1;
}

int
main()
{
#if LUA_VERSION_NUM > 501
	lua_State *L = luaL_newstate();
#else
	lua_State *L = lua_open();
#endif
	luaL_openlibs(L);
#if LUA_VERSION_NUM > 501
	luaL_requiref(L, "mysql", mysql_register, 0);
#else
	mysql_register(L);
#endif
	if(0 != luaL_dofile(L, "mysql.lua"))
	{
		printf("%s\n", lua_tostring(L, -1));
	}
	lua_close(L);
	return 0;
}
