#pragma once
#include "AnyProfile.h"
class DicekiServant :public AnyProfile {
	std::filesystem::path pathDir;
	std::unordered_map<AnyIndex, AnyProfile> chats;
public:
	AnyProfile* getChatProfile(const string& id);
};
