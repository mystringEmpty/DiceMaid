#include "AnysDriver.h"
#include "DicekiServant.h"
#include "AnysLua.h"
#include "FrameAPI.hpp"

void FrameAPI::listenEvent(Anys& raw) {
	auto self = app.getServant(raw["self_bot"]->str());
	try{
		raw.rawset("servant", self);
		if (auto file{ self->getDir() / "event" / (raw["event"]->str() + ".lua") }; fs::exists(file)) {
			AnyLuaState L;
			//api->debug("dofile:" + file.u8string());
			L.dofile(file, raw);
		}
		if (raw["event"] == "message") {
			if (raw["msg"]->str() == "/bot") {
				replyMsg(Any(raw), app.getVer());
			}
			else if (raw["subtype"] == "private_msg") {
				//sendDirectMsg(raw["self_bot"], raw["uid"],"消息已收到√");
			}
			else if (raw["subtype"] == "group_msg") {
				//if (raw["msg"]->str() == "?")sendGroupMsg(raw["self_bot"], raw["gid"], "√");
			}
			return;
		}
		else api->debug("ignored event: " + raw["event"]->str());
	}
	catch (std::exception& e) {
		api->debug(string("listenEvent异常:") + e.what());
	}
}