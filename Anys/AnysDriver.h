#pragma once
#include "AnyProfile.h"
class DicekiServant;
class AnysDriver :public AnyProfileSet {
	std::atomic<bool> Alived = false;
public:
	AnysDriver():AnyProfileSet(getRootDir() / "Diceki", FileType::Toml){}
	virtual AnyClassType type()const { return AnyClassType::Driver; }
	void init();
	[[nodiscard]] bool alive()const { return Alived; }
	void log(const string&, LogLevel)const;
	void debug(const string& info)const { log(info, LogLevel::Debug); }
	void error(const string& info)const { log(info, LogLevel::Error); }
	[[nodiscard]] static const fs::path& getRootDir();
	[[nodiscard]] const string& getVer()const;
	DicekiServant* getServant(const string&);
	void exit();
};

extern AnysDriver app;