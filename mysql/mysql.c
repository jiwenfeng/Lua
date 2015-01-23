#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <mysql.h>

static int
host_is_legal(const char *h)
{
	if(strcmp(h, "localhost") == 0)
	{
		return 0;
	}
	int a, b, c, d;
	sscanf(h, "%d.%d.%d.%d", &a, &b, &c, &d);
	if(a > 0 && a < 255 && b >= 0 && b < 255 && c >= 0 && c < 255 && d > 0 && d < 255)
	{
		return 0;
	}
	return -1;
}

static int
connection_gc(lua_State *L)
{
	MYSQL **mysql = (MYSQL **)lua_touserdata(L, 1);
	if(*mysql)
	{
		mysql_close(*mysql);
	}
	return 0;
}

static int
mysql_connect(lua_State *L)
{
	int narg = lua_gettop(L);
	if(narg != 6)
	{
		lua_pushnil(L);
		lua_pushstring(L, "too few arguments to function 'connect'");
		return 0;
	}
	MYSQL ** mysql = (MYSQL **)lua_touserdata(L, 1);
	luaL_argcheck(L, mysql, 1, "'mysql' excepted ");
	const char *host = lua_tostring(L, 2);
	int ret = host_is_legal(host);
	luaL_argcheck(L, ret == 0, 2, "invalid host address");
	const char *uname = lua_tostring(L, 3);
	const char *passwd = lua_tostring(L, 4);
	const char *dbname = lua_tostring(L, 5);
	int timeout = (int)lua_tonumber(L, 6);
	if(timeout > 0)
	{
		mysql_options(*mysql, MYSQL_OPT_CONNECT_TIMEOUT, (const char *)&timeout);
	}
	if(!mysql_real_connect(*mysql, host, uname, passwd, dbname, 0, NULL, 0))
	{
		lua_pushnil(L);
		lua_pushstring(L, mysql_error(*mysql));
		return 2;
	}
	return 1;
}

static int
mysql_do_it(MYSQL *mysql, const char *str, int length)
{
	return mysql_real_query(mysql, str, length);
}

static int
mysql_table_list(lua_State *L)
{
	MYSQL **mysql = (MYSQL **)lua_touserdata(L, 1);
	luaL_argcheck(L, mysql && *mysql, 1, "'mysql' expected");
	const char *wild = lua_tostring(L, 2);
	MYSQL_RES *res = mysql_list_tables(*mysql, wild);
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
mysql_find(lua_State *L)
{
	MYSQL **mysql = (MYSQL **)lua_touserdata(L, 1);
	luaL_argcheck(L, mysql && *mysql, 1, "'mysql' expected");
	size_t len;
	const char *sql_str = lua_tolstring(L, 2, &len);
	if(mysql_do_it(*mysql, sql_str, len))
	{
		lua_pushnil(L);
		lua_pushstring(L, mysql_error(*mysql));
		return 2;
	}
	MYSQL_RES *res = mysql_store_result(*mysql);
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
mysql_update(lua_State *L)
{
	MYSQL **mysql = (MYSQL **)lua_touserdata(L, 1);
	luaL_argcheck(L, mysql && *mysql, 1, "'mysql' expected");
	size_t len;
	const char *sql_str = lua_tolstring(L, 2, &len); 
	if(mysql_do_it(*mysql, sql_str, len))
	{
		lua_pushnil(L);
		lua_pushstring(L, mysql_error(*mysql));
		return 2;
	}
	lua_pushnumber(L, mysql_affected_rows(*mysql));
	return 1;
}

static int 
mysql_insert(lua_State *L)
{
	MYSQL **mysql = (MYSQL **)lua_touserdata(L, 1);
	luaL_argcheck(L, mysql && *mysql, 1, "'mysql' expected");
	size_t len;
	const char *sql_str = lua_tolstring(L, 2, &len); 
	if(mysql_do_it(*mysql, sql_str, len))
	{
		lua_pushnil(L);
		lua_pushstring(L, mysql_error(*mysql));
		return 2;
	}
	lua_pushnumber(L, mysql_affected_rows(*mysql));
	return 1;
}

static int
mysql_delete(lua_State *L)
{
	MYSQL **mysql = (MYSQL **)lua_touserdata(L, 1);
	luaL_argcheck(L, mysql && *mysql, 1, "'mysql' expected");
	size_t len;
	const char *sql_str = lua_tolstring(L, 2, &len); 
	if(mysql_do_it(*mysql, sql_str, len))
	{
		lua_pushnil(L);
		lua_pushstring(L, mysql_error(*mysql));
		return 2;
	}
	lua_pushnumber(L, mysql_affected_rows(*mysql));
	return 1;
}

static int
mysql_tostring(lua_State *L)
{
	MYSQL **mysql = (MYSQL **)lua_touserdata(L, 1);
	luaL_argcheck(L, mysql && *mysql, 1, "'mysql' expected");
	const char *info = mysql_get_host_info(*mysql);
	lua_pushstring(L, info);
	return 1;
}

static int
connection_new(lua_State *L)
{
	MYSQL ** mysql = (MYSQL **)lua_newuserdata(L, sizeof(MYSQL *));
	*mysql = mysql_init(NULL);
	luaL_getmetatable(L, "mysql");
	lua_setmetatable(L, -2); 
	return 1;
}

static int
mysql_begin_transaction(lua_State *L)
{
	MYSQL **mysql = (MYSQL **)lua_touserdata(L, 1);
	luaL_argcheck(L, mysql && *mysql, 1, "'mysql' expected");
	if(mysql_query(*mysql, "BEGIN"))
	{
		lua_pushnil(L);
		lua_pushstring(L, mysql_error(*mysql));
		return 2;
	}
	return 1;
}

static int
mysql_rollback_transaction(lua_State *L)
{
	MYSQL **mysql = (MYSQL **)lua_touserdata(L, 1);
	luaL_argcheck(L, mysql && *mysql, 1, "'mysql' expected");
	if(mysql_query(*mysql, "ROLLBACK"))
	{
		lua_pushnil(L);
		lua_pushstring(L, mysql_error(*mysql));
		return 2;
	}
	return 1;
}

static int
mysql_commit_transaction(lua_State *L)
{
	MYSQL **mysql = (MYSQL **)lua_touserdata(L, 1);
	luaL_argcheck(L, mysql && *mysql, 1, "'mysql' expected");
	if(mysql_query(*mysql, "COMMIT"))
	{
		lua_pushnil(L);
		lua_pushstring(L, mysql_error(*mysql));
		return 2;
	}
	return 1;

}

static int
mysql_create_table(lua_State *L)
{
	return 0;
}

static int
mysql_drop_table(lua_State *L)
{
	return 0;
}

static int
mysql_drop_db(lua_State *L)
{
	return 0;
}

int
mysql_connection_register(lua_State *L)
{
	static const struct luaL_Reg connection_methods[] = {
		{"connect", mysql_connect},
		{"tostring", mysql_tostring},
		{"query", mysql_find},
		{"insert", mysql_insert},
		{"update", mysql_update},
		{"delete", mysql_delete},
		{"create_table", mysql_create_table},
		{"drop_table", mysql_drop_table},
		{"drop_db", mysql_drop_db},
		{"begin", mysql_begin_transaction},
		{"rollback", mysql_rollback_transaction},
		{"commit", mysql_commit_transaction},
		{"tables", mysql_table_list},
		{NULL, NULL},
	};
	static const struct luaL_Reg connection_class_methods[] = {
		{"New", connection_new},
		{NULL, NULL},
	};
	luaL_newmetatable(L, "mysql");
	luaL_register(L, NULL, connection_methods);

	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	lua_pushcfunction(L, connection_gc);
	lua_setfield(L, -2, "__gc");

	luaL_register(L, "mysql", connection_class_methods);
	return 1;
}

int
main()
{
	lua_State *L = lua_open();
	luaL_openlibs(L);
	mysql_connection_register(L);
	if(-1 == luaL_dofile(L, "mysql.lua"))
	{
		printf("%s\n", lua_tostring(L, -1));
	}
	lua_close(L);
	return 0;
}
