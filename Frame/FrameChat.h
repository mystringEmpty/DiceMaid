#pragma once
#include <unordered_set>
#include "Anys.h"
using std::string;
using std::to_string;
using strview = std::string_view;
using std::unordered_map;
template<typename Elem>
using uset = std::unordered_set<Elem>;

class Chat : public Anys {
protected:
public:
	enum class Type { Unknown, Private, Group };
	Type chatType{ Type::Unknown };
	const long long ID{ 0 };
	//Friend-1;Group-2;Discuss-3
	//virtual int type() { return (int)rawget("type").to_int(); }
	//Chat& type(int i) { rawset("type",i); return *this; }
	/*virtual string tname() {
		static string t{ "Chat" };
		return t;
	}*/
	Chat() = default;
	Chat(long long qq, Type t) :chatType(t), Anys({
		{"ID",to_string(qq)},
		}), ID(qq) {}
	Chat(const char* qq, Type t) :chatType(t), Anys({
		{"ID",qq},
		}), ID(atoll(qq)) {}
	virtual string tname()const override { static const string t{ "Chat" }; return t; }
	operator long long() { return ID; }
	long long to_int()const override { return ID; }
	double to_num()const override { return (double)ID; }
	string str()const override { return rawget("ID").str(); }
};
//Chat* getChat(const lua_var&);

class User :public Chat {
	time_t lastUpdateNick{ 0 };
public:
	//int type()override { return 1; }
	User() = default;
	User(long long qq) :Chat(qq, Type::Private) {}
	User(const char* qq) :Chat(qq, Type::Private) {}
	bool operator==(const User& other)const { return this && ID == other.ID; }

	const string& setNick(const string& nick) {
		if (nick.empty())return rawget("nick");
		lastUpdateNick = time(nullptr);
		rawset("nick", nick);
		return nick;
	}
	//const string& getNick(const lua_var&);

	//string print(const lua_var& bot);
};
//User* getUser(const lua_var&);
class Group :public Chat {
public:
	//int type()override { return 2; }
	string Name;
	Group() = default;
	Group(long long ll) :Chat(ll, Type::Group) {}
	Group(const char* ch) :Chat(ch, Type::Group) {}
	bool operator==(const Group& other)const { return this && ID == other.ID; }
};
Group* getGroup(long long);
//Group* getGroup(const lua_var&);

//Bot Account
class Bot :public Anys {
	AnyIndex ID;
public:
	Bot() = default;
	Bot(AnyIndex id) :ID(id) { }
	string tname()const override { static const string t{ "Bot" }; return t; }
	unordered_map<long long, Chat*> lastChats;

	uset<long long> FriendList;
	//time_t lastGetFriends{ 0 };
	//const uset<long long>& getFriendQQList() { return FriendQQList; };
	//bool isFriend(long long) { return true; };

	/*std::map<long long, lua_var> MsgBackUp;
	std::mutex mtMsgList;
	void backupMsg(long long id, const lua_var& e);
	lua_var lookupMsg(long long id);*/
};
Bot* getBotByID(AnyIndex id);
//Bot* getBot(long long);
//Bot* getBot(const lua_var& bot);
