#include "OneBotAPI.h"
#include "AnysDriver.h"
#include "CharConvert.hpp"
#include "HelperClock.hpp"
#include <unordered_set>
#include <iostream>
//std::unordered_set<long long>approveFriendReq;
//std::unordered_set<long long>eventsFriendAdd;

void OneBotApi::listenEvent(Anys& raw) {
	if (raw["event"] == "message") {
		if (raw["subtype"] == "private_msg") {
			api->debug("处理私聊消息");
			sendDirectMsg(raw["self_bot"], raw["uid"],"消息已收到√");
		}
		else if (raw["subtype"] == "group_msg") {
			if (raw["msg"]->str() == "?")sendGroupMsg(raw["self_bot"], raw["gid"], "√");
		}
		return;
	}
	else api->debug("未监听事件：" + raw["event"]->str());
}

static void OneBot_Event(json& j, ClientState* cli) {
	//clock_t t{ clock() };
	try {
		if (j.count("post_type")) {
			long long botID{ j["self_id"] };
			auto bot{ (OneBot*)getBotByID(botID) };
			if (!bot->client) {
				bot->client = cli;
				if (auto data = cli->get_data({
					{"action","get_login_info"},
					{"echo","login_info"},
					}); !data.is_null()) {
					bot->rawset("name", data["nickname"]);
					api->debug("new Bot:" + string(data["nickname"]));
				}
			}
			if (j.count("meta_event_type"))return;
			if (j["post_type"] == "message") {
				string msg{ j["message"] };
				//频道
				if (j["message_type"] == "guild") {
					string msg_id{ j["message_id"] };
					//QQ::getUser(uid)->setNick(j["sender"]["nickname"]);
					api->listenEvent(Anys{ {
					{"event","message"},
					{"subtype","guild_msg"},
					{"self_bot",bot},
					{"uid",j["user_id"]},
					{"gid",j["guild_id"]},
					{"chid",j["channel_id"]},
					{"msg",msg},
					} });
				}
				else {
					//int msgId{ j["message_id"].get<int>() };
					//私聊
					if (j["message_type"] == "private") {
						api->debug("收到私聊消息");
						api->listenEvent(Anys{ {
						{"event","message"},
						{"subtype","private_msg"},
						{"self_bot",bot},
						{"uid",j["user_id"]},
						{"msg",msg},
						} });
					}
					//群聊
					else if (j["message_type"] == "group") {
						api->listenEvent(Anys{ {
						{"event","message"},
						{"subtype","group_msg"},
						{"self_bot",bot},
						{"uid",j["user_id"]},
						{"gid",j["group_id"]},
						{"msg",msg},
						} });
					}
				}
				return;
			}
			else if (j["post_type"] == "notice") {
				//新好友添加
				if (j["notice_type"] == "friend_add") {
					return;
				}
				//新成员入群
				else if (j["notice_type"] == "group_increase") {
					//long long gid{ j["group_id"] };
					//long long uid{ j["user_id"] };
					return;
				}
				//成员退群
				else if (j["notice_type"] == "group_decrease") {
					if (j["sub_type"] == "leave")return;
					//long long gid{ j["group_id"] };
					//long long uid{ j["user_id"] };
					//long long operatorID{ j["operator_id"] };
					return;
				}
				//群消息撤回
				else if (j["notice_type"] == "group_recall") {
					//long long gid{ j["group_id"] };
					//long long operatorID{ j["operator_id"] };
					//long long targetID{ j["user_id"] };
					//int msgId{ j["message_id"] };
					return;
				}
				//私聊消息撤回
				else if (j["notice_type"] == "friend_recall") {
					long long targetID{ j["user_id"] };
					int msgId{ j["message_id"] };
					//CALLIF(eventExtra, bot, json{ {} }.dump().c_str());
					return;
				}
				else if (j["notice_type"] == "guild_channel_recall") {
					return;
				}
				//其他客户端状态
				else if (j["notice_type"] == "client_status") {
					//QQ::debugLog(string(j["client"]["device_name"]) + ": " + (j["online"] ? "online" : "offline"));
					return;
				}
				//群禁言
				else if (j["notice_type"] == "group_ban") {
					long long gid{ j["group_id"] };
					long long operatorID{ j["operator_id"] };
					long long uid{ j["user_id"] };
					if (j["sub_type"] == "ban") {
					}
					else if (j["sub_type"] == "lift_ban") {
					}
					return;
				}
				//群名片变动
				else if (j["notice_type"] == "group_card") {
					return;
				}
				//群文件上传
				else if (j["notice_type"] == "group_upload") {
					return;
				}
				//群管理变动
				else if (j["notice_type"] == "group_admin") {
					return;
				}
				else if (j["notice_type"] == "essence") {
					return;
				}
				else if (j["notice_type"] == "message_reactions_updated") {
					return;
				}
				//戳一戳
				else if (j["sub_type"] == "poke") {
					//long long uid{ j["user_id"] };
					//long long targetID = j["target_id"].get<long long>();
					return;
				}
				else if (j["sub_type"] == "honor") {
					return;
				}
				else if (j["sub_type"] == "title") {
					return;
				}
				else if (j["sub_type"] == "lucky_king") {
					return;
				}
			}
			else if (j["post_type"] == "message_sent") {
				if (j["message_type"] == "group") {
					//int msgId{ j["message_id"] };
					//long long gid{ j["group_id"] };
					//long long uid = j["self_id"];
					//string msg{ j["message"] };
				}
				else if (j["message_type"] == "private") {
					//long long uid = j["self_id"];
					//if (j["target_id"] != uid)return;
					//int msgId{ j["message_id"] };
					//long long gid{ j["group_id"] };
					//string msg{ j["message"] };
				}
				return;
			}
			else if (j["post_type"] == "request") {
				//群邀请/申请
				if (j["request_type"] == "group") {
					//long long gid = j["group_id"].get<long long>();
					//long long uid = j["user_id"].get<long long>();
					//string seq = j["flag"];
					if (j["sub_type"] == "invite") {
					}
					else if (j["sub_type"] == "add") {
					}
					return;
				}
				else if (j["request_type"] == "friend") {
					//long long uid = j["user_id"];
					//string seq = j["flag"];
					//string msg = j["comment"];
				}
			}
		}
		else if (j["status"] == "ok")return;
	}
	catch (json::exception& e) {
		api->debug(string("解析事件json错误:") + e.what());
	}
	catch (std::exception& e) {
		api->debug(string("事件处理异常:") + e.what());
	}
	api->debug(j.dump(0));
	return;
}
class Event_Pool {
	std::queue<std::pair<json, ClientState*>> eves;
	std::array<std::thread, 10> pool;
	std::mutex Fetching;
	bool isActive{ false };
public:
	void start() {
		if(!isActive)for (int i = 0; i < 10; ++i) {
			pool[i] = std::thread(&Event_Pool::listenEvent, this);
		}
		isActive = true;
	}
	void push(const string& eve, ClientState* cli) {
		std::lock_guard<std::mutex> lk{ Fetching };
		try {
			json j = json::parse(eve); 
			//if (!j.count("meta_event_type")) {
				if (j.count("echo") && !j["echo"].is_null()) {
					cli->mEcho[j["echo"]] = { j["data"],time(nullptr) };
				}
				else eves.emplace(j, cli);
			//}
		}
		catch (json::exception& e) {
			api->debug(string("解析接收json错误:") + e.what());
			api->debug(eve);
		}
	}
	bool fetchEvent(std::pair<json, ClientState*>& eve) {
		std::lock_guard<std::mutex> lk{ Fetching };
		if (!eves.empty()) {
			eve = eves.front();
			eves.pop();
			return true;
		}
		return false;
	}
	void listenEvent() {
		while (isActive) {
			std::pair<json, ClientState*> eve;
			if (fetchEvent(eve)) {
				OneBot_Event(eve.first, eve.second);
				sleep_for(19ms);
			}
			else {
				sleep_for(37ms);
			}
		}
	}
	void end() {
		isActive = false;
		for (auto& th : pool) {
			if (th.joinable())th.join();
		}
		pool = {};
	}
	~Event_Pool() {
		end();
	}
};
Event_Pool eve_pool; 
//lws* client_wsi{ nullptr };
int callback_ws(lws* wsi, enum lws_callback_reasons reasons, void* user, void* _in, size_t _len) {
	ClientState* cli{ (ClientState*)user };
	char buffer[LWS_PRE + MAX_PAYLOAD_SIZE + 1] = { 0 };
	memset(buffer, 0, LWS_PRE + MAX_PAYLOAD_SIZE);
	//发送或者接受buffer，建议使用栈区的局部变量，lws会自己释放相关内存
	//如果使用堆区自定义内存空间，可能会导致内存泄漏或者指针越界
	switch (reasons) {
	case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER: //24
		break;
	case LWS_CALLBACK_CLIENT_HTTP_BIND_PROTOCOL: //85
	case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED: //19
	case LWS_CALLBACK_GET_THREAD_ID: //31
	case LWS_CALLBACK_ADD_POLL_FD: //32
	case LWS_CALLBACK_DEL_POLL_FD: //33
	case LWS_CALLBACK_CHANGE_MODE_POLL_FD: //34
	case LWS_CALLBACK_LOCK_POLL: //35
	case LWS_CALLBACK_UNLOCK_POLL: //36
	case LWS_CALLBACK_PROTOCOL_INIT: //27
	case LWS_CALLBACK_EVENT_WAIT_CANCELLED: //71
	case LWS_CALLBACK_WSI_CREATE: //29
	case LWS_CALLBACK_ESTABLISHED_CLIENT_HTTP: //44
	case LWS_CALLBACK_CLIENT_HTTP_DROP_PROTOCOL: //76
	case LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH: //2 拒绝连接的最后机会
	case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:	//17
	case LWS_CALLBACK_HTTP_CONFIRM_UPGRADE:	//86
	case LWS_CALLBACK_HTTP_BIND_PROTOCOL:	//49
	case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:	//20
	case LWS_CALLBACK_ADD_HEADERS:	//53
		break;
	case LWS_CALLBACK_CONNECTING:
		cout << "Websockets CONNECTING" << endl;
		break;
	case LWS_CALLBACK_ESTABLISHED:	//0
		if (wsi) {
			new(cli) ClientState(wsi);
		}
		eve_pool.start();
		cout << "Websockets ESTABLISHED" << endl;
		lws_callback_on_writable(wsi);
		break;
	case LWS_CALLBACK_CLIENT_ESTABLISHED:   //3
		if (wsi) {
			new(cli) ClientState(wsi);
		}
		eve_pool.start();
		//连接成功时，会触发此reason
		cout << "Websockets client ESTABLISHED" << endl;
		//调用一次lws_callback_on_writeable，会触发一次callback的LWS_CALLBACK_CLIENT_WRITEABLE，之后可进行一次发送数据操作
		lws_callback_on_writable(wsi);
		break;
	case LWS_CALLBACK_CLIENT_RECEIVE_PONG:
		//cout << "Websockets PONG" << endl;
		break;
	case LWS_CALLBACK_WS_CLIENT_DROP_PROTOCOL:	//80
		cout << "Websockets Server Drop." << endl;
		break;
	case LWS_CALLBACK_CLIENT_CLOSED: //75
		// 客户端主动断开、服务端断开都会触发此reason
	case LWS_CALLBACK_CLOSED: //4
		linker->isBreak = true; // ws关闭，发出消息，退出消息循环
		cout << "ws closed." << endl;
		break;
	case LWS_CALLBACK_WSI_DESTROY: //30
		linker->isBreak = true; // ws关闭，发出消息，退出消息循环
		//linker->wsi = nullptr;
		cout << "ws destroyed." << endl;
		break;
	case LWS_CALLBACK_PROTOCOL_DESTROY:	//28
		cout << "protocol destroyed." << endl;
		break;
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:	//1
		//连接失败、异常
		memcpy(buffer, _in, _len);
		cout << "connect error:" << buffer << endl;
		break;
	// 获取到服务端的数据
	case LWS_CALLBACK_CLIENT_RECEIVE:   //8 
	// 获取到客户端的数据
	case LWS_CALLBACK_RECEIVE:	//6
		memcpy(buffer, _in, _len);
		cli->rcvData += buffer;
		if (lws_is_final_fragment(wsi)) {
			//if (_len < MAX_PAYLOAD_SIZE && _len != 4082) {
			static size_t posN{ 0 };
			while ((posN = cli->rcvData.find("\n")) != string::npos) {
				eve_pool.push(cli->rcvData.substr(0, posN), cli);
				cli->rcvData = cli->rcvData.substr(posN + 1);
			}
			if (!cli->rcvData.empty()) {
				eve_pool.push(cli->rcvData, cli);
				cli->rcvData.clear();
			}
		}
		lws_callback_on_writable(wsi);
		break;
	case LWS_CALLBACK_CLIENT_WRITEABLE: //10
		break;
	//调用lws_callback_on_writeable，会触发一次此reason
	case LWS_CALLBACK_SERVER_WRITEABLE: //11
		//if (wsi)linker->wsi = wsi;
		break;
	default:
		cout << "CALLBACK REASON: " << reasons << endl;
		break;
	}
	return 0;
}