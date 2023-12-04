#include <fstream>
#include "AnysDriver.h"
#include "FrameAPI.hpp"
AnysDriver app;
void AnysDriver::init() {
	Alived = true;
}
void AnysDriver::log(const string& info, LogLevel lv) {
	static fs::path logPath(getRootDir() / "Diceki" / "log" / ("log_" + to_string(time(nullptr)) + ".txt"));
	std::ofstream flog(logPath, std::ios::app);
	if (flog)flog << info << std::endl;
	api->log(info, lv);
}

void AnysDriver::exit() {

}