#include "lua.hpp"
#include "CharConvert.hpp"
#include "AnysDriver.h"
#include "AnysLua.h"

static void lua_push_string(lua_State* L, const string& s) {
	lua_pushstring(L, s.c_str());
}
static void lua_push_object(lua_State* L, AnyObject* obj) {
	if (obj) {
		AnyObject** p{ (AnyObject**)lua_newuserdata(L, sizeof(AnyObject*)) };
		obj->copy();
		*p = obj;
		//push_meta
		lua_getglobal(L, "this");
		if (AnyLuaState** val{ (AnyLuaState**)lua_touserdata(L, -1) }) {
			if (auto t{ obj->type() }; MetaList.count(t)) {
				if (!(*val)->registed(obj->type())) {
					(*val)->regist(MetaList[t]);
				}
				luaL_getmetatable(L, MetaList[t].name.c_str());
				lua_setmetatable(L, -3);
			}
		}
		lua_pop(L, 1);
		//luaL_setmetatable(L, get_type()->Name.c_str());
	}
	else lua_pushnil(L);
}
static void lua_push_any(lua_State* L, const Any& var) {
	switch (var.type()) {
	case AnyType::Boolean:
		lua_pushboolean(L, var.to_boolean());
		break;
	case AnyType::Number:
		lua_pushnumber(L, var.to_number());
		break;
	case AnyType::String:
		lua_push_string(L, var.to_string());
		break;
	case AnyType::Table:
	case AnyType::Anys:
		lua_push_object(L, var.as_object());
		break;
	default:
		lua_pushnil(L);
		break;
	}
}

static string lua_to_u8string(lua_State* L, int idx = -1) {
	return luaL_checkstring(L, idx);
}
static string lua_native_to_u8string(lua_State* L, int idx = -1) {
	return charcvtFromNative(luaL_checkstring(L, idx));
}

//lua Object
#define Lua2Obj(i) AnyObject* obj{ *(AnyObject**)luaL_checkudata(L, i, "Object") };
static int lua_Table_index(lua_State* L) {
	Lua2Obj(1);
	lua_push_any(L, obj->get(lua_to_u8string(L, 2)));
	return 1;
}
static int lua_Table_gc(lua_State* L) {
	Lua2Obj(1);
	obj->drop();
	return 0;
}

umap<AnyClassType, AnysClass> MetaList{
	{ AnyClassType::Object,{ AnyClassType::Object, "Object",{
		{"__index",lua_Table_index},}
	}},
};

AnyLuaState::AnyLuaState():AnyObject(AnyType::UserData), L(luaL_newstate()) {
	luaL_openlibs(L);
	//set LuaState
	AnyLuaState** p{ (AnyLuaState**)lua_newuserdata(L, sizeof(AnyLuaState*)) };
	*p = this;
	//luaL_setmetatable(L, "LuaState");
	lua_setglobal(L, "this");
	//add require path
	lua_getglobal(L, "package");
	string strPath((app.getDir() / "lua" / "?.lua").string() + ";"
		+ (app.getDir() / "lua" / "?" / "init.lua").string()
		+ ";?.lua");
	lua_pushstring(L, strPath.c_str());
	lua_setfield(L, -2, "path");
	string strCPath((app.getDir() / "lua" / "?.dll").string()
		+ (app.getDir() / "bin" / "?.dll").string() + ";?.dll");
	lua_pushstring(L, strCPath.c_str());
	lua_setfield(L, -2, "cpath");
	lua_pop(L, 1);
	//app.debug("lua_top:" + std::to_string(lua_gettop(L)));
}
AnyLuaState::~AnyLuaState() {
	lua_close(L);
}
bool AnyLuaState::registed(AnyClassType id) {
	return class_registy.count(id);
}
void AnyLuaState::regist(const AnysClass& meta) {
	luaL_newmetatable(L, meta.name.c_str());
	for (auto& [fname, func] : meta.lua_lib) {
		lua_pushstring(L, fname.c_str());
		lua_pushcfunction(L, func);
		lua_rawset(L, -3);	//tab[key] = val
	}
	lua_setglobal(L, meta.name.c_str());
	class_registy.emplace(meta.type);
}
bool AnyLuaState::dofile(const fs::path& f, const Any& ctx) {
	if (L && fs::exists(f)) {
		if (luaL_loadfile(L, f.string().c_str())) {
			app.error("加载lua文件" + f.filename().u8string() + "失败!\n" + lua_native_to_u8string(L, -1));
			return false;
		}
		lua_push_any(L, ctx);
		lua_setglobal(L, "self");
		if (!lua_pcall(L, 0, 0, 0)) {
			return true;
		}
		app.error("运行lua文件" + f.filename().u8string() + "失败!\n" + lua_native_to_u8string(L, -1));
	}
	else
		app.error("加载lua文件" + f.filename().u8string() + "失败:\n文件不存在!");
	return false;
}