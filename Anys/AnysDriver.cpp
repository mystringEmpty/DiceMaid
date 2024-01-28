#include <fstream>
#include "CharConvert.hpp"
#include "AnysDriver.h"
#include "FrameAPI.hpp"
#include "DicekiServant.h"
AnysDriver app;
void AnysDriver::init() {
	Alived = true;
	fs::create_directories(getRootDir() / "Diceki" / "log");
}
const fs::path& AnysDriver::getRootDir(){
	static fs::path ret = std::filesystem::absolute(std::filesystem::current_path());
	return ret;
}
DicekiServant* AnysDriver::getServant(const string& id) {
	static dict<DicekiServant> ServantList;
	auto tab{ app.get("init")->rawget("servant") };
	string name = tab->incl(id) ? tab[id]->str_or(id) : id;
	if (!ServantList.count(name)) {
		ServantList.emplace(name, DicekiServant(fileDir / "servant" / charcvtToNative(name)));
	}
	return &ServantList.at(name);
}
void AnysDriver::log(const string& info, LogLevel lv)const {
	static fs::path logPath(getRootDir() / "Diceki" / "log" / ("log_" + std::to_string(time(nullptr)) + ".txt"));
	std::ofstream flog(logPath, std::ios::app);
	if (flog)flog << info << std::endl;
	api->log(info, lv);
}

void AnysDriver::exit() {

}