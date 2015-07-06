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
	if(narg != 6)
	{
		lua_pushnil(L);
		lua_pushstring(L, "too few arguments to function 'connect'");
		return 0;
	}
	MYSQL *mysql = *(MYSQL **)lua_touserdata(L, 1);
	luaL_argcheck(L, mysql, 1, "'mysql' excepted ");
	const char *host = lua_tostring(L, 2);
	const char *uname = lua_tostring(L, 3);
	const char *passwd = lua_tostring(L, 4);
	const char *dbname = lua_tostring(L, 5);
	int timeout = (int)lua_tonumber(L, 6);
	if(timeout > 0)
	{
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
mysql_execute(MYSQL *mysql, const char *str, int length)
{
	return mysql_real_query(mysql, str, length);
}

static int
lmysql_tables(lua_State *L)
{
	MYSQL *mysql = *(MYSQL **)lua_touserdata(L, 1);
	luaL_argcheck(L, mysql, 1, "'mysql' expected");
	const char *wild = lua_tostring(L, 2);
	MYSQL_RES *res = mysql_list_tables(mysql, wild);
	if(res)
	{
		MYSQL_ROW row;
		lua_newtable(L);
		int n = 1;
		while((row = mysql_fetch_row(res)))
		{
			lua_pushnumber(L, n++);
			lua_pushstring(L, row[0]);
			lua_settable(L, -3);
		}
	}
	mysql_free_result(res);
	return 1;
}

static int
lmysql_select(lua_State *L)
{
	MYSQL *mysql = *(MYSQL **)lua_touserdata(L, 1);
	luaL_argcheck(L, mysql, 1, "'mysql' expected");
	size_t len;
	const char *sql_str = lua_tolstring(L, 2, &len);
	if(mysql_execute(mysql, sql_str, len))
	{
		lua_pushnil(L);
		lua_pushstring(L, mysql_error(mysql));
		return 2;
	}
	MYSQL_RES *res = mysql_store_result(mysql);
	int nres = mysql_num_rows(res);
	lua_pushnumber(L, nres);
	if(res)
	{
		MYSQL_ROW row;
		MYSQL_FIELD *fields = mysql_fetch_fields(res);
		int nfields = mysql_num_fields(res);
		int i = 0, n = 1;
		lua_newtable(L);
		while((row = mysql_fetch_row(res)))
		{
			lua_pushnumber(L, n++);
			lua_newtable(L);
			for(i = 0; i < nfields; i++)
			{
				lua_pushstring(L, fields[i].name);
				lua_pushstring(L, row[i]);
				lua_settable(L, -3);
			}
			lua_settable(L, -3);
		}
	}
	else
	{
		lua_newtable(L);
		lua_settable(L, -3);
	}
	mysql_free_result(res);
	return 2;
}

static int
lmysql_update(lua_State *L)
{
	MYSQL *mysql = *(MYSQL **)lua_touserdata(L, 1);
	luaL_argcheck(L, mysql, 1, "'mysql' expected");
	size_t len;
	const char *sql_str = lua_tolstring(L, 2, &len); 
	if(mysql_execute(mysql, sql_str, len))
	{
		lua_pushnil(L);
		lua_pushstring(L, mysql_error(mysql));
		return 2;
	}
	lua_pushnumber(L, mysql_affected_rows(mysql));
	return 1;
}

static int 
lmysql_insert(lua_State *L)
{
	MYSQL *mysql = *(MYSQL **)lua_touserdata(L, 1);
	luaL_argcheck(L, mysql, 1, "'mysql' expected");
	size_t len;
	const char *sql_str = lua_tolstring(L, 2, &len); 
	if(mysql_execute(mysql, sql_str, len))
	{
		lua_pushnil(L);
		lua_pushstring(L, mysql_error(mysql));
		return 2;
	}
	lua_pushnumber(L, mysql_affected_rows(mysql));
	return 1;
}

static int
lmysql_delete(lua_State *L)
{
	MYSQL *mysql = *(MYSQL **)lua_touserdata(L, 1);
	luaL_argcheck(L, mysql, 1, "'mysql' expected");
	size_t len;
	const char *sql_str = lua_tolstring(L, 2, &len); 
	if(mysql_execute(mysql, sql_str, len))
	{
		lua_pushnil(L);
		lua_pushstring(L, mysql_error(mysql));
		return 2;
	}
	lua_pushnumber(L, mysql_affected_rows(mysql));
	return 1;
}

static int
lmysql_tostring(lua_State *L)
{
	MYSQL *mysql = *(MYSQL **)lua_touserdata(L, 1);
	luaL_argcheck(L, mysql, 1, "'mysql' expected");
	const char *info = mysql_get_host_info(mysql);
	lua_pushstring(L, info);
	return 1;
}


static int
lmysql_begin(lua_State *L)
{
	MYSQL *mysql = *(MYSQL **)lua_touserdata(L, 1);
	luaL_argcheck(L, mysql, 1, "'mysql' expected");
	if(mysql_query(mysql, "BEGIN"))
	{
		lua_pushnil(L);
		lua_pushstring(L, mysql_error(mysql));
		return 2;
	}
	return 1;
}

static int
lmysql_rollback(lua_State *L)
{
	MYSQL *mysql = *(MYSQL **)lua_touserdata(L, 1);
	luaL_argcheck(L, mysql, 1, "'mysql' expected");
	if(mysql_query(mysql, "ROLLBACK"))
	{
		lua_pushnil(L);
		lua_pushstring(L, mysql_error(mysql));
		return 2;
	}
	return 1;
}

static int
lmysql_commit(lua_State *L)
{
	MYSQL *mysql = *(MYSQL **)lua_touserdata(L, 1);
	luaL_argcheck(L, mysql, 1, "'mysql' expected");
	if(mysql_query(mysql, "COMMIT"))
	{
		lua_pushnil(L);
		lua_pushstring(L, mysql_error(mysql));
		return 2;
	}
	return 1;

}

static int
lmysql_call(lua_State *L)
{
	return 0;
}

static const struct luaL_Reg connection_methods[] = {
	{"connect", lmysql_connect},
	{"tostring", lmysql_tostring},
	{"select", lmysql_select},
	{"insert", lmysql_insert},
	{"update", lmysql_update},
	{"delete", lmysql_delete},
	{"begin", lmysql_begin},
	{"rollback", lmysql_rollback},
	{"commit", lmysql_commit},
	{"tables", lmysql_tables},
	{"call", lmysql_call},
	{NULL, NULL},
};

static const struct luaL_Reg connection_class_methods[] = {
	{"New", lmysql_open},
	{NULL, NULL},
};

int
mysql_connection_register(lua_State *L)
{
	luaL_newmetatable(L, "mysql");
#if LUA_VERSION_NUM > 501
	luaL_setfuncs(L, connection_methods, 0);
#else
	luaL_register(L, NULL, connection_methods);
#endif
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	lua_pushcfunction(L, lmysql_close);
	lua_setfield(L, -2, "__gc");

#if LUA_VERSION_NUM > 501
	luaL_setfuncs(L, connection_class_methods, 0);
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
	luaL_requiref(L, "mysql", mysql_connection_register, 1);
#else
	mysql_connection_register(L);
#endif
	if(0 != luaL_dofile(L, "mysql.lua"))
	{
		printf("%s\n", lua_tostring(L, -1));
	}
	lua_close(L);
	return 0;
}
