#pragma once
#include <iostream>
#include "WSServer.hpp"
#include "WSClient.hpp"
#include "FrameAPI.hpp"

using namespace std::this_thread;
using namespace std::chrono_literals;
using std::cin;
using std::cout;
using std::endl;

extern std::unique_ptr<WSLinker> linker;
//extern std::unordered_map<AnyIndex, lws*> clients;
class OneBot :public Bot {
public:
	OneBot() = default;
	OneBot(const AnyIndex& id):Bot(id){}
	ClientState* client = nullptr;
};
//extern std::unordered_map<AnyIndex, Bot> bots;
//Bot* getBot(AnyIndex id);

class OneBotApi :public FrameAPI {
	std::unordered_map<long long, Anys> MsgBackUp;
	std::queue<long long> MsgIDQueue;
	std::mutex mtMsgList;
public:
	OneBotApi() = default;
	//~OneBotApi();
	//群-申请人
	//unordered_map<long long, unordered_map<long long, Anys>> GroupReqs;
	//被邀请bot-群
	//unordered_map<long long, unordered_map<long long, Anys>> GroupInvs;
	//被邀请bot-申请者QQ
	//unordered_map<long long, unordered_map<long long, Anys>> FriendReqs;
	//void listenEvent(Anys&)override;
	std::optional<Anys> lookupMsg(Any id) {
		std::lock_guard<std::mutex> lock{ mtMsgList };
		if (auto i{ id.to_int() }; MsgBackUp.count(id.to_int()))
			return MsgBackUp[i];
		else return std::nullopt;
	}
	void updateStrangerInfo(const Any& bot, long long id) {
		bot.as_anys<OneBot>()->client->send(json{
			{"action","get_stranger_info"},
			{"params",{
				{"user_id",id},
			}},
			{"echo","stranger_info"},
		});
	}
	//override
	void log(const string& msg, LogLevel lv)override {
		static string heads[7] = { "Debug", "Info", "Notice", "Warning", "Error", "Critical", "Fatal"};
		cout << '[' << heads[(int)lv] << "] " << charcvtToNative(msg) << endl;
	}
	void sendMsg(const Any& bot, const Any& aim, const string& msg)override;
	void sendDirectMsg(const Any& bot, const Any& aim, const string& msg)override;
	void sendGroupMsg(const Any& bot, const Any& aim, const string& msg)override;
	void replyMsg(const Any& eve, const string& msg)override;
	void eraseMsg(const Any& bot, const Any& msg)override {
		if (msg && msg.incl("msg_id")) {
			bot.as_anys<OneBot>()->client->send(json{
				{"action","delete_msg"},
				{"params",{
					{"message_id",msg.is_object()
					? msg.as_anys()->rawget("msg_id")->to_int()
					: msg.to_int()},
				}}, }
			);
		}
	}
	void quitGroup(const Any& self, const Any& grp)override {
		if (!self || !grp)return;
	}
	void groupMute(const Any& bot, const Any& grp, const Any& aim, const Any& dur)override {
		if (!bot || !grp)return;
		bot.as_anys<OneBot>()->client->send(json{
			{"action","set_group_ban"},
			{"params",{
				{"group_id",grp.to_int()},
				{"user_id",aim.to_int()},
				{"duration",dur.to_int()},
			}}, }
		);
	}
	void groupKick(const Any& bot, const Any& grp, const Any& aim, bool isNever)override {
		if (!bot || !grp)return;
		bot.as_anys<OneBot>()->client->send(json{
			{"action","set_group_kick"},
			{"params",{
				{"group_id",grp.to_int()},
				{"user_id",aim.to_int()},
				{"reject_add_request",isNever},
				//{"reject_add_request",isNever ? "true" : "false"},
			}}, }
		);
	}
	bool groupNotice(const Any& bot, const Any& grp, const string& title, const string& content)override {
		if (!bot || !grp)return false;
		bot.as_anys<OneBot>()->client->send(json{
			{"action","_send_group_notice"},
			{"params",{
				{"group_id",grp.to_int()},
				{"content",content},
			}}, }
		);
		return true;
	}

	void answerGroupRequest(Anys& req, int approve, const string& ret) {
		if (!req.incl("bot") || !req.incl("actor") || !req.incl("group"))return;
		std::this_thread::sleep_for(3s);
		req["bot"]->as_anys<OneBot>()->client->send(json{
			{"action","set_group_add_request"},
			{"params",{
				{"flag",req.rawget("flag")->str()},
				{"type","add"},
				{"approve",approve == 1},
				{"reason",ret},
			}}, }
		);
		if (approve == 1 && !ret.empty()) {
			std::this_thread::sleep_for(3s);
			sendGroupMsg(req["bot"], req["group"], ret);
		}
	}
	void answerGroupInvite(Anys& req, int approve, const string& ret) {
		if (req.incl("bot") && req.incl("actor") && req.incl("group")) {
			std::this_thread::sleep_for(3s);
			req["bot"]->as_anys<OneBot>()->client->send(json{
				{"action","set_group_add_request"},
				{"params",{
					{"flag",req["flag"]->str()},
					{"type","invite"},
					{"approve",approve == 1},
					{"reason",ret},
				}}, }
			);
			if (approve == 1 && !ret.empty()) {
				std::this_thread::sleep_for(3s);
				sendGroupMsg(req["bot"], req["group"], ret);
			}
		}
	}
	void answerFriendRequest(Anys& req, int approve, const string& ret) {
		if (!req.incl("bot")|| !req.incl("actor"))return;
		req["bot"]->as_anys<OneBot>()->client->send(json{
			{"action","set_friend_add_request"},
			{"params",{
				{"flag",req.rawget("flag")},
				{"approve",approve == 1},
			}}, }
		);
		if (approve == 1 && !ret.empty()) {
			std::this_thread::sleep_for(3s);
			sendDirectMsg(req.rawget("bot"), req.rawget("actor"), ret);
		};
	}
};
