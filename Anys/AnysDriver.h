#pragma once
#include "AnyProfile.h"
class AnysDriver :public AnyProfile {
	std::atomic<bool> Alived = false;
public:
	std::unordered_map<AnyIndex, AnyProfile> configs;
	void init();
	bool alive()const { return Alived; }
	void log(const string&, LogLevel);
	void debug(const string& info) { log(info, LogLevel::Debug); }
	void error(const string& info) { log(info, LogLevel::Error); }
	const fs::path& getRootDir()const;
	void exit();
};

extern AnysDriver app;