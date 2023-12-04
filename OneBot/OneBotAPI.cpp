#include <filesystem>
#include "OneBotAPI.h"
#include "AnysDriver.h"
#include "AppInfo.h"
#include "CharConvert.hpp"
//#include "HelperChar.hpp"
#include "HelperClock.hpp"
#include "WSServer.hpp"
#include "toml.hpp"

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
const fs::path& AnysDriver::getRootDir()const {
	static fs::path ret = std::filesystem::absolute(std::filesystem::current_path());
	return ret;
}
//const string Empty;

string ws_url;
bool link_server() {
	linker = std::make_unique<WSServer>(ws_url);
	return true;
}

bool driver_init() {
	app.init();
	if (fs::path tomlInit{ app.getRootDir() / "Diceki" / "init.toml" }; fs::exists(tomlInit)) {
		cout << tomlInit << endl;
		auto t = toml::parse_file(tomlInit.u8string());
		if (t["onebot"]["ws_address"]) {
			ws_url = string(*t["onebot"]["ws_address"].as_string());
			cout << "ws_address:" << ws_url << endl;
			return true;
		}
	}
	api->fatal("服务端配置读取失败！请确认Diceki/init.toml有写入ws_address！");
	return false;
}

int main() {
	if (driver_init()) {
		while (link_server()) {
			linker->keep();
			sleep_for(30s);
		}
	}
	app.exit();
	return 0;
}
