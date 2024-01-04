#pragma once
#include "AnyProfile.h"
class AnysDriver :public AnyProfileSet {
	std::atomic<bool> Alived = false;
public:
	AnysDriver():AnyProfileSet(std::filesystem::absolute(std::filesystem::current_path()) / "Diceki", FileType::Toml){}
	void init();
	bool alive()const { return Alived; }
	void log(const string&, LogLevel);
	void debug(const string& info) { log(info, LogLevel::Debug); }
	void error(const string& info) { log(info, LogLevel::Error); }
	const fs::path& getRootDir()const;
	void exit();
};

extern AnysDriver app;