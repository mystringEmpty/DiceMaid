#include "FrameChat.h"
#include "FrameAPI.hpp"
#include "lua_var.h"
#include "lua_mod.hpp"
#include "lua.hpp"
#include "DicekiConsole.h"

unordered_map<long long, User> users;
User* getUser(const lua_var& user) {
	if (user.as<User>())return user.as<User>();
	if (long long id{ user.is_table() ? user["ID"]->to_int() : user.to_int() }) {
		if (!users.count(id))users.emplace(id, User(id));
		return &users[id];
	}
	else return nullptr;
}
string User::print(const lua_var& bot) {
	return "[" + getNick(bot) + "](" + getID() + ")";
}

unordered_map<long long, Group> grps;
Group* getGroup(long long id) {
	if (!id)return nullptr;
	if (!grps.count(id))grps.emplace(id, Group(id, 2));
	return &grps[id];
}
Group* getGroup(const lua_var& chat) {
	if (chat.as<Group>())return chat.as<Group>();
	else if (chat.is_table())return getGroup(chat["ID"]->to_int());
	return getGroup(chat.to_int());
}
Group* getDiscuss(long long id) {
	if (!id)return nullptr;
	if (!grps.count(id))grps.emplace(id, Group(id, 3));
	return &grps[id];
}
Chat* getChat(const lua_var& chat) {
	if (chat.as<Chat>())return chat.as<Chat>();
	long long id(chat["ID"]->to_int());
	if (id)switch (chat["type"]->to_int()) {
	case 1:
		return getUser(id);
	case 2:
		return getGroup(id);
	case 3:
		return getDiscuss(id);
	}
	return nullptr;
}

unordered_map<long long, Bot> bots;
Bot* getBot(long long id) {
	if (!id)return nullptr;
	if (!bots.count(id)) {
		bots.emplace(id, Bot(id));
	}
	return &bots[id];
}
Bot* getBot(const lua_var& bot) {
	if (bot.as<Bot>())return bot.as<Bot>();
	return getBot(bot.to_int());
}
/*
lua_var Chat_index_ID(lua_var self) {
	return self.to_udata<Chat>()->getID();
}
lua_var Chat_index_Type(lua_var self) {
	return self.to_udata<Chat>()->type();
	//Chat* chat{ self.to_udata<Chat>() };
	//return 	chat->type() == 1 ? "User" : (chat->type() == 2 ? "Group" : "Discuss");
}*/
/*
lua_var Chat_to_table(lua_var self) {
	if (self.is_table())return self;
	if (!self || !self.is_udata()) return lua_var::new_table();
	Chat* chat{ self.to_udata<Chat>() };
	return lua_var((lua_table*)chat);
}*/
lua_var Chat_from_table(lua_var self) {
	return lua_var(getChat(self));
}

//Chat Method
int User_get(lua_State* L) {
	lua_var id{ L,1 };
	lua_var user{ getUser(id.to_int())};
	user.push(L);
	return 1;
};
int Group_get(lua_State* L) {
	lua_var id{ L,1 };
	lua_var grp{ getGroup(id.to_int()) };
	grp.push(L);
	return 1;
};

//Bot Library
int Bot_get(lua_State* L) {
	lua_var id{ L,1 };
	lua_var bot{ getBot(id),"Bot" };
	bot.push(L);
	return 1;
};
int Bot_sendMsg(lua_State* L) {
	lua_var bot{ L, 1 };
	if (!bot)return 0;
	lua_var aim{ L, 2 };
	lua_var msg{ L, 3 };
	frame->SendMsg(bot, aim, msg);
	return 0;
}
int Bot_sendDirect(lua_State* L) {
	lua_var bot{ L, 1 };
	if (!bot)return 0;
	lua_var aim{ L, 2 };
	lua_var msg{ L, 3 };
	frame->SendDirectMsg(bot, aim, msg);
	return 0;
}
int Bot_quitGroup(lua_State* L) {
	lua_var bot{ L, 1 };
	lua_var grp{ L, 2 };
	frame->QuitGroup(bot, grp);
	return 0;
}
int Bot_groupGag(lua_State* L) {
	lua_var bot{ L, 1 };
	lua_var grp{ L, 2 };
	lua_var aim{ L, 3 };
	lua_var dur{ L, 4 };
	frame->GroupGag(bot, grp, aim, dur);
	return 0;
}
int Bot_groupKick(lua_State* L) {
	lua_var bot{ L, 1 };
	lua_var grp{ L, 2 };
	lua_var aim{ L, 3 };
	lua_var never{ L, 4 };
	frame->GroupKick(bot, grp, aim, bool(never));
	return 0;
}
int Bot_groupNotice(lua_State* L) {
	lua_var bot{ L, 1 };
	lua_var grp{ L, 2 };
	lua_var title{ L, 3 };
	lua_var content{ L, 4 };
	lua_pushboolean(L, frame->GroupNotice(bot, grp, title, content));
	return 1;
}
int Bot_getUserData(lua_State* L) {	//#1 udata(Bot)/string/number:bot;#2 string;udata(Chat)/string/number:user
	lua_var bot{ L, 1 };
	if (!bot)return 0;
	lua_var user{ L, 2 };
	if (!user)return 0;
	console.bot(bot).getUserData(user)->push(L);
	return 1;
}
int Bot_getGroupData(lua_State* L) {	//#1 udata(Bot)/string/number:bot;#2 string;udata(Chat)/string/number:user
	lua_var bot{ L, 1 };
	if (!bot)return 0;
	lua_var chat{ L, 2 };
	if (!chat)return 0;
	console.bot(bot).getGroupData(chat)->push(L);
	return 1;
}
int Bot_getConf(lua_State* L) {	//#1 udata(Bot)/string/number:bot;#2 string;udata(Chat)/string/number:user
	Bot* bot{ getBot(lua_var(L,1)) };
	if (!bot)return 0;
	Servant& self{ console.bot(bot) };
	lua_var chat{ L, 2 };
	if (!chat)self.conf().push(L);
	else if (long long ct{ chat["type"]->to_int() }; ct == 1) {
		self.getUserData(chat)->push(L);
	}
	else{
		self.getGroupData(chat)->push(L);
	}
	return 1;
}

size_t resChat = lua_mod::regist(
	lua_mod("Chat")
	.asTable()
	.metalib({
	{"getUser", User_get},
	{"getGroup", Group_get}, })
	.indexlib({})
	/*.index({
	{"ID",Chat_index_ID },
	{"type",Chat_index_Type }, })*/
.fromtable(Chat_from_table)
	);

int Bot_lookupMsg(lua_State* L) {	//#1 udata(Bot)/string/number:bot;#2 string:msg_id
	lua_var id{ L, 2 };
	frame->lookupMsg(id.to_int()).push(L);
	return 1;
}

size_t resbot = lua_mod::regist(
	lua_mod("Bot")
	.meta("Chat")
	.asTable()
	.metalib({
	{"get",Bot_get}, })
	.indexlib({ 
	{"sendMsg",Bot_sendMsg},
	{"sendDirect",Bot_sendDirect},
	{"quitGroup", Bot_quitGroup},
	{"gag", Bot_groupGag},
	{"kick", Bot_groupKick},
	{"notice", Bot_groupNotice},
	{"conf", Bot_getConf},
	{"confUser", Bot_getUserData},
	{"confGroup", Bot_getGroupData},
	{"getMsgById",Bot_lookupMsg}, })
.fromtable([](lua_var var) {
	return lua_var(getBot(var));
})
	);