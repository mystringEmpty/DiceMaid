#include <filesystem>
#include "OneBotAPI.h"
#include "AnysDriver.h"
#include "AppInfo.h"
#include "CharConvert.hpp"
//#include "HelperChar.hpp"
#include "HelperClock.hpp"
#include "WSServer.hpp"

template<typename T>
using ptr = std::shared_ptr<T>;

std::unique_ptr<FrameAPI> api{ std::make_unique<OneBotApi>() };
std::unique_ptr<WSLinker> linker;
//std::unordered_map<AnyIndex, lws*> clients;
Bot* getBotByID(AnyIndex id) {
	static std::unordered_map<AnyIndex, OneBot> bots;
	if (!bots.count(id))bots.emplace(id, OneBot(id));
	return &bots[id];
}

const char* _DriverVer() {
	static string ver = string(app_title) + " by " + app_author + " ver" + app_ver + "(" + std::to_string(app_build) + ")[" + _built_time() + "] for OneBot";
	return ver.c_str();
}
//const string Empty;


bool driver_init() {
	app.init();
	if (auto& cfg{ *app.get("init") };cfg.incl("onebot")) {
		if (string ws_url = cfg["onebot"]["ws_address"]->str(); !ws_url.empty()) {
			cout << "ws_address:" << ws_url << endl;
			linker = std::make_unique<WSServer>(ws_url);
			return true;
		}
	}
	api->fatal("服务端配置读取失败！请确认Diceki/init.toml有写入ws_address！");
	return false;
}

int main() {
	if (driver_init()) {
		linker->keep();
	}
	app.exit();
	return 0;
}
