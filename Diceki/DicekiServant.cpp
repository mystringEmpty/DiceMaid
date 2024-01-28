#include "DicekiServant.h"
AnyProfile* DicekiServant::getChatProfile(const string& id) {
	if (!chats.count(id)) {
		chats.emplace(id, pathDir / "chat" / (id + ".toml"));
	}
	return &chats[id];
}