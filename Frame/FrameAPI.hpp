#pragma once
/**
 * 框架接口抽象类
 * 2022-08-25
 */
#include <memory>
#include <mutex>
#include <queue>
#include <unordered_set>
#include "Anys.h"
using std::shared_ptr;
using std::make_shared;
template<typename T>
using uset = std::unordered_set<T>;

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
	//Chat() = default;
	Chat(long long qq, Type t) :chatType(t), Anys({
		{"ID",std::to_string(qq)},
		}), ID(qq) {}
	//virtual string tname()const override { static const string t{ "Chat" }; return t; }
	operator long long() { return ID; }
	long long to_int()const override { return ID; }
	double to_num()const override { return (double)ID; }
	string str()const override { return rawget("ID").str(); }
};
//Chat* getChat(const lua_var&);
class User :public Chat {
	time_t lastUpdateNick{ 0 };
public:
	AnyClassType type()const override { return AnyClassType::User; }
	User() = default;
	User(long long qq) :Chat(qq, Type::Private) {}
	//User(const char* qq) :Chat(qq, Type::Private) {}
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
class Group :public Chat {
public:
	AnyClassType type()const override { return AnyClassType::Group; }
	string Name;
	//Group() = default;
	Group(long long ll) :Chat(ll, Type::Group) {}
	//Group(const char* ch) :Chat(ch, Type::Group) {}
	bool operator==(const Group& other)const { return this && ID == other.ID; }
};
Group* getGroup(long long);
//Group* getGroup(const lua_var&);

class Bot :public Anys {
protected:
	AnyIndex ID;
public:
	Bot() = default;
	Bot(AnyIndex id) :ID(id) {
		fields.insert("ID", id.str());
	}
	//string tname()const override { static const string t{ "Bot" }; return t; }
	string str()const override { return ID.str(); }
	AnyIndex to_index()const override { return ID; }
	umap<long long, Chat*> lastChats;

	uset<AnyIndex> FriendList;
	uset<AnyIndex> GroupList;
	//time_t lastGetFriends{ 0 };
	//const uset<long long>& getFriendQQList() { return FriendQQList; };
	//bool isFriend(long long) { return true; };

	/*std::map<long long, lua_var> MsgBackUp;
	std::mutex mtMsgList;
	void backupMsg(long long id, const lua_var& e);
	lua_var lookupMsg(long long id);*/
};
Bot* getBotByID(AnyIndex id);

class FrameAPI {
protected:
	std::mutex mtMsgList;
public:
	virtual bool IsEnable() { return true; };
	virtual void log(const string& msg, LogLevel lv = LogLevel::Info) {}
	void debug(const string& msg) { log(msg, LogLevel::Debug); }
	void error(const string& msg) { log(msg, LogLevel::Error); }
	void fatal(const string& msg) { log(msg, LogLevel::Fatal); }
	virtual void listenEvent(Anys&);

	virtual User* getUser(AnyIndex id) {
		static std::unordered_map<AnyIndex, User> UserList;
		if (!UserList.count(id))UserList.emplace(id, User(id.to_ll()));
		return &UserList.at(id);
	}
	virtual Group* getGroup(AnyIndex id) {
		static std::unordered_map<AnyIndex, Group> GroupList;
		if (!GroupList.count(id))GroupList.emplace(id, Group(id.to_ll()));
		return &GroupList.at(id);
	}

	virtual int isFriend(const Any&, const Any&) { return true; }
	virtual void sendMsg(const Any&, const Any&, const string&) {}
	virtual void sendDirectMsg(const Any&, const Any&, const string&) {}
	virtual void sendGroupMsg(const Any&, const Any&, const string&) {}
	virtual void replyMsg(const Any&, const string&) {}
	virtual void eraseMsg(const Any&, const Any&) {}
	virtual void quitGroup(const Any&, const Any&) {}
	virtual void groupMute(const Any&, const Any&, const Any&, const Any&) {}
	virtual void groupKick(const Any&, const Any&, const Any&, bool = false) {}
	virtual bool groupNotice(const Any& bot, const Any& grp, const string&, const string&) { return false; }
	//答复加群申请
	//virtual void answerGroupRequest(Any&, int approve, const string& rep = "") {}
	//答复入群邀请
	//virtual void answerGroupInvite(Any&, int approve, const string& rep = "") {}
	//答复好友申请
	//virtual void answerFriendRequest(Any&, int approve, const string& rep = "") {}
	//virtual const std::vector<long long>& GetOnlineBots() { return {}; }
	virtual std::optional<Anys> lookupMsg(Any id) { return std::nullopt; }
	//virtual void backupMsg(long long id, const Any& e) {}
};
extern std::unique_ptr<FrameAPI> api; //one&only; inited when Library is loaded;