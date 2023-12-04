#include "lua.hpp"
#include "DicekiLua.h"
#include "lua_mod.hpp"
#include "DicekiConsole.h"
#include "lua_var.h"
#include "FrameAPI.hpp"
#include "FrameChat.h"

//事件依赖库
int event_reply(lua_State* L) {//#1 ctable(udata):event #2 string:msg
	lua_var eve{ LUA_VAR_TABLE(L, 1, Event) };
	//if(!eve["bot"])return 0;
	string msg{ lua_var(L, 2) };
	console.bot(eve["bot"]).reply(eve, msg);
	return 0;
}

int event_approve(lua_State* L) {//#1 table:event #2 number:resType #3 number:resText
	int argc{ lua_gettop(L) };
	lua_var eve{ L, 1 };
	string type{ eve["type"] };
	int ret{ (int)lua_tonumber(L, 2) };
	string text{ argc > 2 ? lua_tostring(L, 3) : "" };
	if (type == "FriendRequest") {
		//frame->DebugLog("处理好友申请");
		frame->AnswerFriendRequest(eve, ret, text);
	}
	else if (type == "GroupInvited") {
		//frame->DebugLog("处理入群邀请,来自" + eve["actor"]["ID"]->to_string());
		frame->AnswerGroupInvite(eve, ret, text);
	}
	else if (type == "GroupRequest") {
		//frame->DebugLog("处理加群申请");
		frame->AnswerGroupRequest(eve, ret, text);
	}
	else frame->DebugLog("处理未知申请");
	return 0;
}

int event_erase(lua_State* L) {//#1 table:event
	int argc{ lua_gettop(L) };
	lua_var eve{ L, 1 };
	if (eve["type"] != "Message")return 0;
	lua_var bot{ eve["bot"] };
	frame->EraseMsg(bot, eve);
	return 0;
}

int event_block(lua_State* L) {//#1 ctable(udata):event
	lua_var eve{ LUA_VAR_TABLE(L, 1, Event) };
	eve["is_block"] = true;
	return 0;
}
int event_ignore(lua_State* L) {//#1 ctable(udata):event
	lua_var eve{ LUA_VAR_TABLE(L, 1, Event) };
	eve["is_ignored"] = true;
	return 0;
}

size_t res_event = lua_mod::regist(
	lua_mod("Event")
	.asTable()
	.metalib({})
	.indexlib({
	 {"reply",event_reply},
	 {"approve",event_approve},
	 {"erase",event_erase},
	 {"block",event_block},
	 {"ignore",event_ignore}, })
	.index({
	{"uid",[](lua_var self) { return lua_var(self["actor"]["ID"]); }},
	{"gid",[](lua_var self) { return lua_var(self["group"]["ID"]); }},
	{"sid",[](lua_var self) { return lua_var(self["bot"]["ID"]); }},
	{"tid",[](lua_var self) { return lua_var(self["taker"]["ID"]); }},
	{"nick",[](lua_var self) { 
	User* user{ getUser(self["actor"])};
	return user ? lua_var(user->getNick(self["bot"])) : lua_var();
}},
	{"me",[](lua_var self) {
	Bot* bot{getBot(self["bot"])};
	lua_var me{ console.bot(bot).conf().getfield("call_self") };
	return me ? me
		: (bot ? lua_var(bot->getNick(self["bot"])) : lua_var());
}},
	{"Lua",[](lua_var self) {
		lua_var lua = lua_var::new_state();
		lua_state* state{ lua.as_state() };
		if (!state)return lua_var();
		state->open().setglobal("event", self.set_type("Event"));
		return lua_var(self["Lua"] = lua);
}}, })
);