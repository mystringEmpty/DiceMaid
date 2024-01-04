#include <fstream>
#include "AnysDriver.h"
#include "FrameAPI.hpp"
AnysDriver app;
void AnysDriver::init() {
	Alived = true;
	fs::create_directories(getRootDir() / "Diceki" / "log");
}
const fs::path& AnysDriver::getRootDir()const {
	static fs::path ret = std::filesystem::absolute(std::filesystem::current_path());
	return ret;
}
void AnysDriver::log(const string& info, LogLevel lv) {
	static fs::path logPath(getRootDir() / "Diceki" / "log" / ("log_" + to_string(time(nullptr)) + ".txt"));
	std::ofstream flog(logPath, std::ios::app);
	if (flog)flog << info << std::endl;
	api->log(info, lv);
}

void AnysDriver::exit() {

}