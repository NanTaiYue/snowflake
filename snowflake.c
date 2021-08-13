/* 
* snowflake算法C实现及lua绑定
*/
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#define INDEX_VAL     0x0ffff             
#define WORKID_VAL    0x03ff       
#define TIMESTAMP_VAL 0x01ffffffffff

struct SF {
	uint64_t last_timestamp;
	uint16_t workid;
	uint16_t index;
};

static struct SF g_state = {0, 0, 0};

static uint64_t 
get_timestamp() {
	struct timeval tv;
	gettimeofday(&tv, 0);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static void 
wait_next_second() {
	uint64_t current_timestamp = 0;
	do {
		current_timestamp = get_timestamp();
	} while(g_state.last_timestamp >= current_timestamp);
	g_state.last_timestamp = current_timestamp;
	g_state.index = 0;
}

static uint64_t
next_id() {
	uint64_t current_timestamp = get_timestamp();
	if (current_timestamp == g_state.last_timestamp) {
		if (g_state.index < INDEX_VAL) {
			++g_state.index;
		} else {
			wait_next_second();
		}
	} else {
		g_state.last_timestamp = current_timestamp;
		g_state.index = 0;
	}
	uint64_t ret = (uint64_t)(
		(g_state.last_timestamp & TIMESTAMP_VAL) << 22 |
		(g_state.workid & WORKID_VAL) << 10 |
		(g_state.index & INDEX_VAL));
	return ret;
}

static int 
init(uint16_t id) {
	if (g_state.workid != 0) {
		return 1;
	}
	g_state.workid = id;
	g_state.index = 0;
	return 0;
}

static int
linit(lua_State* L) {
	uint16_t w_id = 0;
	if (lua_gettop(L) > 0) {
		lua_Integer id = luaL_checkinteger(L, 1);
		if (id < 0 || id > WORKID_VAL) {
			return luaL_error(L, "id out of 1023");
		}
		w_id = (uint16_t)id;
	}
	if (init(w_id)) {
		return luaL_error(L, "workid has been inited");
	}
	lua_pushboolean(L, 1);
	return 1;
}

static int
lnextid(lua_State* L) {
	uint64_t id = next_id();
	lua_pushinteger(L, (lua_Integer)id);
	return 1;
}

LUAMOD_API int 
luaopen_snowflake(lua_State* L) {
	luaL_checkversion(L);
	luaL_Reg lib[] = {
		{"init", linit},
		{"gen", lnextid},
		{NULL, NULL},
	};
	luaL_newlib(L, lib);
	return 1;
}