#pragma once
/**
 * 框架接口抽象类
 * 2022-08-25
 */
#include <string>
#include <memory>
#include <mutex>
#include <queue>
#include "Anys.h"
using std::string;
using std::to_string;
using std::shared_ptr;
using std::make_shared;

class FrameAPI {
protected:
	std::mutex mtMsgList;
public:
	virtual bool IsEnable() { return true; };
	virtual void log(const string& msg, LogLevel lv = LogLevel::Info) {}
	void debug(const string& msg) { log(msg, LogLevel::Debug); }
	void error(const string& msg) { log(msg, LogLevel::Error); }
	void fatal(const string& msg) { log(msg, LogLevel::Fatal); }
	virtual void listenEvent(Anys&) {};
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