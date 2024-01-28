#include <filesystem>
#include "CharConvert.hpp"
#include "HelperClock.hpp"
#include "AnysDriver.h"
#include "OneBotAPI.h"
#include "AppInfo.h"

template<typename T>
using ptr = std::shared_ptr<T>;

std::unique_ptr<FrameAPI> api{ std::make_unique<OneBotApi>() };
std::unique_ptr<WSLinker> linker;
//std::unordered_map<AnyIndex, lws*> clients;
Bot* getBotByID(AnyIndex id) {
	static umap<AnyIndex, OneBot> bots; 
	if (!bots.count(id))bots.emplace(id, OneBot(id));
	return &bots[id];
}

const string& AnysDriver::getVer()const {
	static string ver = string(app_title) + " by " + app_author
		+ " ver" + app_ver + "(" + std::to_string(app_build) + ")[" + _built_time() + "] for " + app.get("init")->rawget("onebot")["impl"]->str_or("OneBot");
	return ver;
}
//const string Empty;

static bool driver_init() {
	app.init();
	if (auto& cfg{ *app.get("init") };cfg.incl("onebot")) {
		if (string ws_url = cfg["onebot"]["ws"]->str(); !ws_url.empty()) {
			cout << "ws_address:" << ws_url << endl;
			linker = std::make_unique<WSClient>(ws_url);
			return true;
		}
		else if (ws_url = cfg["onebot"]["reverse_ws"]->str(); !ws_url.empty()) {
			cout << "reverse_ws:" << ws_url << endl;
			linker = std::make_unique<WSServer>(ws_url);
			return true;
		}
	}
	api->fatal("服务端配置读取失败！请确认Diceki/init.toml有写入onebot.ws或reverse_ws！");
	return false;
}

int main() {
	if (driver_init()) {
		linker->keep();
	}
	app.exit();
	return 0;
}

//OneBotApi
void OneBotApi::sendMsg(const Any& bot, const Any& aim, const string& msg){
	//Bot* self{ getBot(bot) };
	//Chat* chat{ aim.to_udata<Chat>() };
	if (bot && aim && !msg.empty()) {
		if (auto ct{ aim.as_anys<Chat>()->chatType }; ct == Chat::Type::Private) {
			sendDirectMsg(bot, aim, msg);
		}
		else if (ct == Chat::Type::Group) {
			sendGroupMsg(bot, aim, msg);
		}
	}
}
void OneBotApi::sendDirectMsg(const Any& bot, const Any& aim, const string& msg){
	if (bot && aim && !msg.empty()) {
		api->debug("[U]>>" + msg);
		bot.as_anys<OneBot>()->client->send(json{
			{"action","send_private_msg"},
			{"params",{
				{"user_id",aim.to_int()},
				//{"group_id",self->lastChats[aimQQ]->ID},
				{"message",msg},
			}}, }
		);
	}
	else if (!bot)api->error("sendDirectMsg: bot is null!");
	else if (!aim)api->error("sendDirectMsg: aim is null!");
}
void OneBotApi::sendGroupMsg(const Any& bot, const Any& aim, const string& msg){
	if (bot && aim && !msg.empty()) {
		api->debug("[G]>>" + msg);
		bot.as_anys<OneBot>()->client->send(json{
			{"action","send_group_msg"},
			{"params",{
				{"group_id",aim.to_int()},
				{"message",msg},
			}}, }
		);
	}
}
void OneBotApi::replyMsg(const Any& eve, const string& msg){
	if (msg.empty())return;
	//string subtype{ eve["subtype"] };
	//int msgtype{ (int)eve["raw_type"]->to_int() };
	if (auto chat{ eve["chat"]->as_anys<Chat>() }) {
		if (chat->type() == AnyClassType::Group) {
			sendGroupMsg(eve["self_bot"], chat, msg);
		}
		else {
			//if (subtype == "self")msgtype = 1;
			sendDirectMsg(eve["self_bot"], chat, msg);
		}
	}
	else {
		api->error("Reply获取聊天ID失败");
		return;
	}
}