#pragma once
#include "AnyProfile.h"
class DicekiServant :public AnyProfile {
	fs::path pathDir;
	std::unordered_map<AnyIndex, AnyProfile> chats;
public:
	DicekiServant(fs::path& dir) :pathDir(dir), AnyProfile(dir.string() + ".toml") {}
	const fs::path& getDir()const { return pathDir; }
	AnyProfile* getChatProfile(const string& id);
};
